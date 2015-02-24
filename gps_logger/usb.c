/*
 * usb.c
 *
 * Created: 26/11/2014 16:33:40
 *  Author: paul.qureshi
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdarg.h>
#include <stdio.h>

#include "global.h"
#include "version.h"
#include "hw.h"
#include "crypto.h"
#include "gps.h"
#include "usb.h"


/**************************************************************************************************
** Send null terminated string of chars from program flash memory. Returns as soon as the last
** character is in the FIFO. Does not send the terminating null.
*/
void USB_print_P(const __flash char *string)
{
	while (*string != '\0')
		USB_tx_char(*string++);
}

/**************************************************************************************************
** Send null terminated string of chars from SRAM. Returns as soon as the last character is in
** the FIFO. Does not send the terminating null.
*/
void USB_print(char *string)
{
	while (*string != '\0')
		USB_tx_char(*string++);
}

/**************************************************************************************************
** Wrapper for printf() that outputs to the terminal
*/
void USB_printf_P(PGM_P format, ...)
{
	char	temp_string[127];
	
	va_list args;
	va_start(args, format);
	vsprintf_P(temp_string, format, args);
	va_end(args);
	USB_print(temp_string);
}

/**************************************************************************************************
** Send a uint8 as 8 binary digits
*/
void USB_tx_bin8(uint8_t bin)
{
	uint8_t		i;

	for (i = 0; i < 8; i++)
	{
		if ((bin & (1<<7)) != 0)
			USB_tx_char('1');
		else
			USB_tx_char('0');
		bin <<= 1;
	}
}

/**************************************************************************************************
** Simple USB terminal
*/
void USB_terminal(void)
{
	uint32_t	rx = 0;
	bool		clear_rx = false;
	
	for(;;)
	{
		rx <<= 8;
		rx |= udi_cdc_getc();
		clear_rx = true;
		
		switch(rx)
		{
			// device info
			case LE_CHR('i', 'n', 'f', 'o'):
				USB_printf_P(PSTR("GPS Logger V%u.%u\r\n"), VERSION_MAJOR, VERSION_MINOR);
				USB_print_P(PSTR("FW built: " __DATE__ " " __TIME__ "\r\n"));
				USB_printf_P(PSTR("Last reset:\t%02X "), HW_last_reset_status);
				if (HW_last_reset_status & RST_SRF_bm)
					USB_print_P(PSTR("SR "));
				if (HW_last_reset_status & RST_PDIRF_bm)
					USB_print_P(PSTR("PDI "));
				if (HW_last_reset_status & RST_WDRF_bm)
					USB_print_P(PSTR("WDR "));
				if (HW_last_reset_status & RST_BORF_bm)
					USB_print_P(PSTR("BOR "));
				if (HW_last_reset_status & RST_EXTRF_bm)
					USB_print_P(PSTR("EXT "));
				if (HW_last_reset_status & RST_SRF_bm)
					USB_print_P(PSTR("POR"));
				USB_newline();
				break;
			
			case LE_CHR('k', 'e', 'y', '='):
				{
					uint8_t	i;
					uint8_t	key[AES_KEY_LENGTH];
					
					for (i = 0; i < AES_KEY_LENGTH; i++)
						key[i] = udi_cdc_getc();
					
					CRYPT_generate_intermediate_key(key);
					break;
				}

			case LE_CHR('s', 'a', 'v', 'e'):
				{
					uint8_t	i;
					uint8_t	key[AES_KEY_LENGTH];
					
					for (i = 0; i < AES_KEY_LENGTH; i++)
						key[i] = udi_cdc_getc();
					
					CRYPT_load_key(key);
					break;
				}
			
			case LE_CHR('d', 'u', 'm', 'p'):
				GPS_dump();
				break;

			default:
				clear_rx = false;
				break;
		}
		
		if (clear_rx)
			rx = 0;
	}
}