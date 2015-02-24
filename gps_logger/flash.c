/*
 * flash.c
 *
 * Created: 19/11/2014 15:34:19
 *  Author: paul.qureshi
 */ 

#include <avr/io.h>
#include <util/delay.h>

#include "global.h"
#include "eeprom.h"
#include "flash.h"


uint32_t	fl_next_page;


/**************************************************************************************************
** Low level SPI byte send+receive
*/
inline uint8_t fl_spi(uint8_t byte)
{
	FL_SPI.DATA = byte;								// start transaction
	while ((FL_SPI.STATUS & SPI_IF_bm) == 0)		// wait for completion
		;
	return (FL_SPI.DATA);
}

/**************************************************************************************************
** Init interface and flash memory
*/
void FL_init(void)
{
	EEP_EnableMapping();
	fl_next_page = *(uint32_t *)EEP_MAPPED_ADDR(0, 0);
	EEP_DisableMapping();
	if (fl_next_page >= FL_NUM_PAGES)
		fl_next_page = 0;

	FL_SPI.INTCTRL = 0;
	FL_SPI.CTRL = SPI_CLK2X_bm | SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc;
}

/**************************************************************************************************
** Shut down safely, saving fl_next_page to EEPROM
*/
void FL_shutdown(void)
{
	EEP_LoadPageBuffer((uint8_t *)&fl_next_page, sizeof(fl_next_page));
	EEP_AtomicWritePage(0);
}

/**************************************************************************************************
** Write buffer to a flash page. Buffer must be <= 512 bytes long.
*/
void FL_write(const void *buffer, uint16_t buffer_length, uint32_t page)
{
	
}

/**************************************************************************************************
** Write buffer to the next flash page to use
*/
void FL_write_next_page(const void *buffer, uint16_t buffer_length)
{
	FL_write(buffer, buffer_length, fl_next_page * 512);
	fl_next_page++;
	if (fl_next_page >= FL_NUM_PAGES)
		fl_next_page = 0;
}
