/*
 * fram.c
 *
 * Created: 25/11/2014 14:11:06
 *  Author: paul.qureshi
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "global.h"
#include "hw.h"
#include "fram.h"


/**************************************************************************************************
** Set up FRAM after reset
*/
void FRAM_init(void)
{
	FRAM_PORT.OUTSET = FRAM_CS_PIN_bm | FRAM_WP_PIN_bm | FRAM_HOLD_PIN_bm;
	FRAM_PORT.DIRSET = FRAM_CS_PIN_bm | FRAM_WP_PIN_bm | FRAM_HOLD_PIN_bm;
	
	// set up write protection
	FRAM_EN;
	HW_spi(FRAM_CMD_WREN);
	FRAM_DIS;

	FRAM_EN;
	HW_spi(FRAM_CMD_WRSR);
	HW_spi(FRAM_STATUS_WEL_bm);		// disable all protection
	FRAM_DIS;
}

/**************************************************************************************************
** Write to FRAM
*/
void FRAM_write(uint16_t address, const void *buffer, uint16_t buffer_size)
{
	uint8_t	*p = (uint8_t *)buffer;
	
	FRAM_EN;
	HW_spi(FRAM_CMD_WRITE);
	HW_spi(address >> 8);
	HW_spi(address & 0xFF);
	
	while (buffer_size--)
		HW_spi(*p++);
	FRAM_DIS;
}

/**************************************************************************************************
** Read from FRAM
*/
void FRAM_read(uint16_t address, void *buffer, uint16_t buffer_size)
{
	uint8_t	*p = (uint8_t *)buffer;
	
	FRAM_EN;
	HW_spi(FRAM_CMD_READ);
	HW_spi(address >> 8);
	HW_spi(address & 0xFF);

	while (buffer_size--)
		*p++ = HW_spi(0x00);
	FRAM_DIS;
}

/**************************************************************************************************
** Write atomic buffer
*/
void FRAM_atomic_write(const void *buffer, uint16_t buffer_size)
{
	FRAM_ATOMIC_METADATA_t	meta;
	uint8_t	*p = (uint8_t *)buffer;
	
	CRC.CTRL = CRC_RESET_RESET1_gc;
	NOP();
	CRC.CTRL = CRC_CRC32_bm | CRC_SOURCE_IO_gc;

	FRAM_EN;
	HW_spi(FRAM_CMD_WRITE);
	HW_spi(FRAM_MAP_ATOMIC_DATA >> 8);
	HW_spi(FRAM_MAP_ATOMIC_DATA & 0xFF);
	
	while (buffer_size--)
	{
		CRC.DATAIN = *p;
		HW_spi(*p++);
	}
	CRC.STATUS = CRC_BUSY_bm;
	FRAM_DIS;

	meta.marker = FRAM_ATOMIC_MARKER;
	meta.crc32 = (uint32_t)CRC.CHECKSUM0 | ((uint32_t)CRC.CHECKSUM1 << 8) | ((uint32_t)CRC.CHECKSUM2 << 16) | ((uint32_t)CRC.CHECKSUM3 << 24);
	FRAM_write(FRAM_MAP_ATOMIC_META+2, &meta.crc32, sizeof(meta.crc32));
	FRAM_write(FRAM_MAP_ATOMIC_META, &meta.marker, sizeof(meta.marker));
}

/**************************************************************************************************
** Check atomic buffer
*/
bool FRAM_atomic_check(uint16_t buffer_size)
{
	FRAM_ATOMIC_METADATA_t	meta;
	uint32_t	crc_checksum = 0;

	CRC.CTRL = CRC_RESET_RESET1_gc;
	NOP();
	CRC.CTRL = CRC_CRC32_bm | CRC_SOURCE_IO_gc;

	FRAM_EN;
	HW_spi(FRAM_CMD_READ);
	HW_spi(FRAM_MAP_ATOMIC_DATA >> 8);
	HW_spi(FRAM_MAP_ATOMIC_DATA & 0xFF);
	
	while (buffer_size--)
		CRC.DATAIN = HW_spi(0x00);
	CRC.STATUS = CRC_BUSY_bm;
	FRAM_DIS;

	crc_checksum = (uint32_t)CRC.CHECKSUM0 | ((uint32_t)CRC.CHECKSUM1 << 8) | ((uint32_t)CRC.CHECKSUM2 << 16) | ((uint32_t)CRC.CHECKSUM3 << 24);
	FRAM_read(FRAM_MAP_ATOMIC_META, &meta, sizeof(meta));

	if (meta.crc32 == crc_checksum)
		return(true);
	return(false);
}

/**************************************************************************************************
** Clear atomic buffer
*/
void FRAM_atomic_clear(void)
{
	FRAM_ATOMIC_METADATA_t	meta;
	memset(&meta, 0, sizeof(meta));
	
	FRAM_write(FRAM_MAP_ATOMIC_META, &meta, sizeof(meta));
}