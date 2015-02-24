/*
 * dataflash.c
 * Author:	Paul Qureshi
 * Created: 18/11/2011 15:39:04
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "global.h"
#include "hw.h"
#include "terminal.h"
#include "dataflash.h"


#pragma region Low Level

/**************************************************************************************************
** Send a number of bytes
*/
void DF_spi_buffer(void *buffer, uint16_t buffer_size)
{
	uint8_t *p = (uint8_t *)buffer;
	
	while (buffer_size--)
		HW_spi(*p++);
}

/**************************************************************************************************
** Send a page/byte address. Does not validate the page/byte.
*/
inline void df_spi_addr(uint16_t page, uint16_t byte)
{
	// 264 byte pages
	HW_spi((page >> 7) & 0xFF);
	HW_spi(((page << 1) | (byte >> 8)) & 0xFF);
	HW_spi(byte & 0xFF);
}

/**************************************************************************************************
** Set up Dataflash, enter low power state
*/
void DF_init(void)
{
	DF_PORT.OUTSET = DF_RES_PIN_bm | DF_CS_PIN_bm;

	// reset Dataflash
	DF_PORT.OUTCLR = DF_RES_PIN_bm;
	_delay_ms(1);
	DF_PORT.OUTSET = DF_RES_PIN_bm;
	_delay_ms(20);		// maximum time before writing is possible

	// wait for DF to become ready
	uint16_t	error_counter = 0;
	while (!DF_ready())
	{
		error_counter++;
		_delay_us(1);
		if (error_counter > 60000)
		{
			TERM_print_P(PSTR("DF_init(): Dataflash not responding.\r\n"));
			//HW_boot_failure(HW_BOOT_FAILURE_DATAFLASH);
			for(;;);
		}
	}

	// check device ID
	DF_DEVID_t devid;
	uint8_t *p = (uint8_t *)&devid;
	DF_EN;
	HW_spi(DF_CMD_RD_DEVID);
	for (uint8_t i = 0; i < sizeof(devid); i++)
		*p++ = HW_spi(0x00);
	DF_DIS;

	if ((devid.manufacturer_id != 0x1F) ||		// JEDEC assigned code for Adesto
		(devid.device_id_1 != 0x28) ||			// AT45Dxxx family, 64 mega power
		(devid.device_id_2 != 0x00))			// standard series
	{
		TERM_printf_P(PSTR("DF_init(): Incorrect Dataflash ID (%02X %02X %02X)\r\n"),
						devid.manufacturer_id, devid.device_id_1, devid.device_id_2);
		for(;;);
	}
}

/**************************************************************************************************
** Check if Dataflash is ready for next command by examining the status register.
*/
inline bool df_ready(void)
{
	uint8_t status;

	DF_EN;
	HW_spi(DF_CMD_STATUS);
	status = HW_spi(0x00);
	DF_DIS;

	if (status & (1<<7))	// bit 7 is RDY/BUSY
		return(true);		// 1 = ready
	else
		return(false);		// 0 = busy
}

/**************************************************************************************************
** Check status register to see if DF is ready to execute further commands.
*/
inline void DF_wait_for_ready(void)
{
	while (!df_ready())
		;
}

#pragma endregion

#pragma region Read / Write / Erase

/**************************************************************************************************
** Erase an entire page. Returns success or failure, but can only really fail if the Dataflash is
** busy for an extended time.
*/
void DF_erase_page(uint16_t page)
{
	DF_wait_for_ready();

	DF_EN;
	HW_spi(DF_CMD_PAGE_ERASE);
	df_spi_addr(page, 0);
	DF_DIS;
}

/**************************************************************************************************
** Erase an entire Sector. Returns success or failure, but can only really fail if the Dataflash is
** busy for an extended time. Note that sectors 0a and 0b cannot be erased with this function.
*/
void DF_erase_sector(uint8_t sector)
{
	if (sector == 0)
		return;

	DF_wait_for_ready();

	DF_EN;
	HW_spi(DF_CMD_SECTOR_ERASE);
	df_spi_addr((uint16_t)sector * 256, 0);
	DF_DIS;
}

/**************************************************************************************************
** Start / end a continuous array read.
**
** Once started bytes can simply be clocked out with HW_spi(0x00) and the address is auto incremented.
*/
void DF_start_array_read(uint16_t page, uint16_t byte)
{
	DF_wait_for_ready();

	DF_EN;
	HW_spi(DF_CMD_ARRAY_READ);
	df_spi_addr(page, byte);
	HW_spi(0x00);				// dummy byte, see datasheet
}

/**************************************************************************************************
** Start / end writing a buffer.
**
** buffer2 == DF_BUF1 / DF_BUF2
** byte is byte to start writing from (address auto-increments)
**
** Can fail if DF is busy.
*/
void DF_start_buffer_write(bool buffer2, uint16_t byte)
{
	DF_wait_for_ready();

	DF_EN;
	if (!buffer2)
		HW_spi(DF_CMD_BUF1_WRITE);
	else
		HW_spi(DF_CMD_BUF2_WRITE);
	df_spi_addr(0, byte);
}

/**************************************************************************************************
** Start / end writing a buffer. Same as DF_start_buffer_write() but does not check the busy flag.
** This is useful if you want to do double buffered writes by filling one buffer while the other
** one is commited to flash memory with DF_write_buffer();
*/
void DF_start_buffer_write_no_busy_check(bool buffer2, uint16_t byte)
{
	DF_EN;
	if (!buffer2)
		HW_spi(DF_CMD_BUF1_WRITE);
	else
		HW_spi(DF_CMD_BUF2_WRITE);
	df_spi_addr(0, byte);
}

/**************************************************************************************************
** Write buffer to flash memory page.
**
** buffer2 == DF_BUF1 / DF_BUF2
** erase enables built in page erase function (takes an extra 34ms)
*/
void DF_write_buffer(bool buffer2, bool erase, uint16_t page)
{
	DF_DIS;		// no need to call DF_end_buffer_write()

	DF_wait_for_ready();

	DF_EN;

	if (!buffer2)
	{
		if (erase)
			HW_spi(DF_CMD_WRITE_BUF1_W_ERASE);
		else
			HW_spi(DF_CMD_WRITE_BUF1_NO_ERASE);
	}
	else
	{
		if (erase)
			HW_spi(DF_CMD_WRITE_BUF2_W_ERASE);
		else
			HW_spi(DF_CMD_WRITE_BUF2_NO_ERASE);
	}

	df_spi_addr(page, 0);
	DF_DIS;
}

#pragma endregion

#pragma region Diagnostics / development

/**************************************************************************************************
** Dumps a page from Dataflash to terminal. Debug only. Doesn't validate parameters or check for
** errors.
*/
void DF_print_page(uint16_t page)
{
	uint16_t	byte = 0;
	uint8_t		i, j;

	DF_start_array_read(page, 0);
	for (j = 0; j < 8; j++)		// 32 x 8 = 256 bytes
	{
		TERM_printf_P(PSTR("0x%02X:\t"), byte);	// print address

		for (i = 0; i < 32; i++)							// print one line of data
			TERM_printf_P(PSTR("%02X"), HW_spi(0x00));
		TERM_newline();

		byte += 32;
	}

	// last 8 odd bytes
	TERM_print_P(PSTR("0xFF:\t"));
	for (i = 0; i < 8; i++)
		TERM_printf_P(PSTR("%02X"), HW_spi(0x00));

	DF_end_array_read();
	TERM_newline();
}

/**************************************************************************************************
** Wipes the entire Dataflash memory.
**
** Note that this does not update any of the log data and will thus break log related stuff until
** logs are either erased or the logger is reset.
**
** Used only for factory reset, takes several seconds to complete.
*/
void DF_erase_entire_memory(void)
{
	// chip erase command sequence
	DF_EN;
	HW_spi(0xC7);
	HW_spi(0x94);
	HW_spi(0x80);
	HW_spi(0x9A);
	DF_DIS;

	// wait for it to finish
	do
	{
		_delay_ms(1000);
		TERM_tx_char('.');
		WDR();
	} while (!DF_ready());

	TERM_newline();
}

#pragma endregion