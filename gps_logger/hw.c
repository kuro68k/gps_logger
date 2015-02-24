/*
 * hw.c
 *
 * Created: 19/11/2014 16:48:15
 *  Author: paul.qureshi
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#include "global.h"
#include "hw.h"

/*
FUSES = {
	0xFF,		// fusebyte 0
	0x00,		// fusebyte 1
	0xBD,		// fusebyte 2
	0xFF,		// fusebyte	3
	0xFF,		// fusebyte 4
	0xD2,		// fusebyte 5
};
*/

uint8_t	HW_last_reset_status;


/**************************************************************************************************
** Set initial hardware state after power-on
*/
void HW_init(void)
{
	HW_last_reset_status = RST.STATUS;
	RST.STATUS = 0xFF;		// clear all reset flags
}

/**************************************************************************************************
** Write a CCP protected register. Registers protected by CCP require the CCP register to be written
** first, followed by writing the protected register within 4 instruction cycles.
**
** Interrupts are temporarily disabled during the write. Code mostly adapted/stolen from Atmel's
** clksys_driver.c and avr_compiler.h.
*/
void HW_CCPWrite(volatile uint8_t *address, uint8_t value)
{
        uint8_t	saved_sreg;

        // disable interrupts if running
		saved_sreg = SREG;
		cli();
		
		volatile uint8_t * tmpAddr = address;
        RAMPZ = 0;

        asm volatile(
                "movw r30,  %0"       "\n\t"
                "ldi  r16,  %2"       "\n\t"
                "out   %3, r16"       "\n\t"
                "st     Z,  %1"       "\n\t"
                :
                : "r" (tmpAddr), "r" (value), "M" (CCP_IOREG_gc), "i" (&CCP)
                : "r16", "r30", "r31"
                );

        SREG = saved_sreg;
}

/**************************************************************************************************
** Spend and receive one byte via shared SPI bus
*/
uint8_t	HW_spi(uint8_t tx)
{
	SPI.DATA = tx;								// start transaction
	while ((SPI.STATUS & SPI_IF_bm) == 0)		// wait for completion
		;
	return (SPI.DATA);
}