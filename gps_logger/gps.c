/*
 * gps.c
 *
 * Created: 18/11/2014 16:09:21
 *  Author: paul.qureshi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "global.h"
#include "hw.h"
#include "terminal.h"
#include "dataflash.h"
#include "crypto.h"
#include "usb.h"
#include "gps.h"


char		buffer[256];
uint16_t	write_head_AT = 0;
uint16_t	read_head = 0;

char		sentence[127];
uint8_t		sentence_length = 0;
char		*param[20];
uint8_t		param_count = 0;

GPS_DATA_t	flash_buffer[BUFFERED_RECORDS];
uint8_t		flash_buffer_index = 0;
uint16_t	df_page = 0;
uint32_t	df_page_counter = 0;


/**************************************************************************************************
** RX interrupt handler, simple circular buffer
*/
ISR(USARTC0_RXC_vect)
{
	buffer[write_head_AT++] = GPS_USART.DATA;
	if (write_head_AT >= sizeof(buffer))
		write_head_AT = 0;
}

/**************************************************************************************************
** Set up GPS and USART
*/
void gps_init(void)
{
	// set up hardware
	memset(flash_buffer, 0xFF, sizeof(flash_buffer));
	
	GPS_PORT.DIRSET = GPS_TX_PIN_bm;
	GPS_PORT.DIRCLR = GPS_RX_PIN_bm;
	
	int		bsel = 1651;
	uint8_t	bscale = -4;
	GPS_USART.BAUDCTRLA = (uint8_t)bsel;
	GPS_USART.BAUDCTRLB = (bscale << 4) | (bsel	>> 8);
	GPS_USART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
	GPS_USART.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
	GPS_USART.CTRLA = USART_RXCINTLVL_MED_gc;
	
	
	// find next position to store records
	TERM_print_P(PSTR("Finding oldest record..."));
	
	uint32_t	counter;
	uint16_t	page;
	for (page = 0; page < DF_NUM_PAGES; page++)
	{
		WDR();
		
		DF_start_array_read(page, 0);
		counter = HW_spi(0x00);
		counter |= (uint32_t)HW_spi(0x00) << 8;
		counter |= (uint32_t)HW_spi(0x00) << 16;
		counter |= (uint32_t)HW_spi(0x00) << 24;
		DF_end_array_read();
		
		if (counter == 0xFFFFFFFF)	// erased page
		{
			df_page = page;			// use this page
			break;
		}
		
		if (counter > df_page_counter)
		{
			df_page = page + 1;		// use the next page
			df_page_counter = counter;
		}
	}
	
	if (df_page >= DF_NUM_PAGES)	// in case last page in flash was the highest, or other error
		df_page = 0;

	TERM_printf_P(PSTR("Staring from page %u, count %lu\r\n"), df_page, df_page_counter);
}

/**************************************************************************************************
** Get a GPS sentence
*/
void gps_get_sentence(void)
{
	sentence_length = 0;
	
	for(;;)
	{
		char c;
		
		do {
			// check for new characters
			c = '\0';
			cli();
			if (write_head_AT != read_head)
				c = buffer[read_head++];
			sei();
			if (read_head > sizeof(buffer))
				read_head = 0;
		
			if (c != '\0')
			{
				if (c == '\n')								// NMEA EOL is \r\n
				{
					sentence[sentence_length] = '\0';
					return;
				}

				sentence[sentence_length++] = c;
				if (sentence_length >= sizeof(sentence))	// overflow
				{
					sentence[sentence_length-1] = '\0';
					return;
				}
			}
		} while (c != '\0');
		
		_delay_ms(1);
	}
}

/**************************************************************************************************
** Returns a pointer to the requested parameter in a comma separated list, or NULL if not found
*/
void gps_decode_parameters(void)
{
	char *c = sentence;
	
	param_count = 0;
	while ((c = strchr(c, ',')) != NULL)
	{
		c++;
		param[param_count] = c;
		param_count++;
	}
}

/**************************************************************************************************
** Decode two digits
*/
uint8_t gps_decode_dd(char *dd)
{
	uint8_t	n;
	
	if ((*dd < '0') || (*dd > '9'))
		return(0);
	n = *dd - '0';
	n *= 10;
	
	dd++;

	if ((*dd < '0') || (*dd > '9'))
		return(0);
	n += *dd - '0';

	return(n);
}

/**************************************************************************************************
** Add a record to the flash buffer
*/
void gps_add_to_flash_buffer(GPS_DATA_t *rec)
{
	memcpy(&flash_buffer[flash_buffer_index], rec, sizeof(GPS_DATA_t));
	flash_buffer_index++;
	if (flash_buffer_index >= BUFFERED_RECORDS)
	{
		GPS_PAGE_HEADER_t	header;
		uint8_t				*p = (uint8_t *)flash_buffer;
		uint16_t			i;
		
		// encrypt data
		//CRYPT_process_buffer(flash_buffer, sizeof(flash_buffer), false);
		
		// header + CRC
		header.counter = df_page_counter++;
		CRC.CTRL = CRC_RESET_RESET1_gc;
		NOP();
		CRC.CTRL = CRC_CRC32_bm | CRC_SOURCE_IO_gc;
		for (i = 0; i < sizeof(flash_buffer); i++)
			CRC.DATAIN = *p++;
		CRC.STATUS = CRC_BUSY_bm;
		header.crc32 = (uint32_t)CRC.CHECKSUM0 | ((uint32_t)CRC.CHECKSUM1 << 8) | ((uint32_t)CRC.CHECKSUM2 << 16) | ((uint32_t)CRC.CHECKSUM3 << 24);

		// write to flash
		DF_start_buffer_write(DF_BUF1, sizeof(GPS_PAGE_HEADER_t));
		DF_spi_buffer(&header, sizeof(header));
		DF_spi_buffer(&flash_buffer, sizeof(flash_buffer));
		DF_end_buffer_write();
		DF_write_buffer(DF_BUF1, true, df_page++);
		if (df_page >= DF_NUM_PAGES)
			df_page = 0;
		
		memset(flash_buffer, 0xFF, sizeof(flash_buffer));
	}
}

/**************************************************************************************************
** Dump all records to USB
*/
void GPS_dump(void)
{
	USB_print_P(PSTR("END\r\n"));
}

/**************************************************************************************************
** Decode GPS output
*/
void GPS_run(void)
{
	GPS_DATA_t	rec;
	
	TERM_printf_P(PSTR("Record:\t%u\r\n"), sizeof(GPS_DATA_t));
	TERM_printf_P(PSTR("Buffer:\t%u\r\n"), sizeof(flash_buffer));
	
	gps_init();
	
	for(;;)
	{
		WDR();
		gps_get_sentence();
		if (sentence_length < 10)
			continue;
		if (strncmp_P(sentence, PSTR("$GP"), 3) != 0)
			continue;

		TERM_print(sentence);
		TERM_newline();
		
		gps_decode_parameters();
		
		// decode known sentences
		if (strncmp_P(&sentence[3], PSTR("VTG"), 3) == 0)
		{
			//		  true    mag     knots   kph
			// $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
			//		  0		1 2		3 4		5 6		7
			if (param_count < 8)
				continue;
			rec.kph = strtod(param[6], NULL);
		}
		
		if (strncmp_P(&sentence[3], PSTR("RMC"), 3) == 0)
		{
			//		  time		 AV lat      NS lon      EW knts true   date   mag
			// $GPRMC,112326.000,A,5053.7018,N,00104.1420,W,0.20,256.69,261112,,,A*7D
			//		  0			 1 2         3 4          5 6    7      8      9 10
			if (param_count < 11)
				continue;



			// time
			rec.hr = gps_decode_dd(param[0]);
			rec.min = gps_decode_dd(param[0] + 2);
			rec.sec = gps_decode_dd(param[0] + 4);

			// date
			rec.d = gps_decode_dd(param[8]);
			rec.m = gps_decode_dd(param[8] + 2);
			rec.y = gps_decode_dd(param[8] + 4);

			// validate data/time
			TERM_printf_P(PSTR("%02u/%02u/%02u %02u:%02u:%02u "), rec.d, rec.m, rec.y, rec.hr, rec.min, rec.sec);
			if ((rec.d > 31) || (rec.m > 12) || (rec.y > 99) ||
				(rec.y < 14) || (rec.y > 79) ||							// year resets to 1980 if unset
				(rec.hr > 23) || (rec.min > 59) || (rec.sec > 59))
			{
				TERM_print_P(PSTR("Bad date/time\r\n"));
				continue;
			}
			
			if (*param[1] != 'A')	// Active = GPS locked
			{
				TERM_print_P(PSTR("No lock\r\n"));
				continue;
			}


			
			// location
			char *p1, *p2;
			p1 = strchr(param[2], '.');
			p2 = strchr(param[4], '.');
			if ((p1 == NULL) || (p2 == NULL) ||
				(*(param[2]+1) == ',') || (*(param[4]+1) == ','))
			{
				TERM_print_P(PSTR("Bad lat/lon\r\n"));
				continue;
			}
			
			rec.lat_left = strtoul(param[2], NULL, 10);
			rec.lat_right = strtoul(p1, NULL, 10);
			rec.lon_left = strtoul(param[4], NULL, 10);
			rec.lon_right = strtoul(p2, NULL, 10);
		}
	}
}

