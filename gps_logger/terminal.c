/*
 * terminal.c
 *
 * Created: 19/11/2014 11:42:36
 *  Author: paul.qureshi
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdarg.h>
#include <stdio.h>

#include "global.h"
#include "terminal.h"


/**************************************************************************************************
** Set up terminal
*/
void TERM_init(void)
{
	TERM_PORT.DIRSET = TERM_TX_PIN_bm;
	TERM_PORT.DIRCLR = TERM_RX_PIN_bm;
	
	int		bsel = 150;		// 115200 baud
	uint8_t	bscale = -7;

	TERM_USART.CTRLB |= USART_CLK2X_bm;
	TERM_USART.BAUDCTRLA = (uint8_t) bsel;
	TERM_USART.BAUDCTRLB = (bscale << 4) | (bsel >> 8);
	TERM_USART.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
}

/**************************************************************************************************
** Send single char. Note that this function returns as soon as the character is in the FIFO and it
** will take some time to be transmitted.
*/
void TERM_tx_char(char c)
{
	while ((TERM_USART.STATUS & USART_DREIF_bm) == 0)	// wait for a free FIFO slot
		;
	TERM_USART.DATA = c;
}

/**************************************************************************************************
** Send null terminated string of chars from program flash memory. Returns as soon as the last
** character is in the FIFO. Does not send the terminating null.
*/
void TERM_print_P(const __flash char *string)
{
	while (*string != '\0')
		TERM_tx_char(*string++);
}

/**************************************************************************************************
** Send null terminated string of chars from SRAM. Returns as soon as the last character is in
** the FIFO. Does not send the terminating null.
*/
void TERM_print(char *string)
{
	while (*string != '\0')
		TERM_tx_char(*string++);
}

/**************************************************************************************************
** Wrapper for printf() that outputs to the terminal
*/
void TERM_printf_P(PGM_P format, ...)
{
	char	temp_string[127];
	
	va_list args;
	va_start(args, format);
	vsprintf_P(temp_string, format, args);
	va_end(args);
	TERM_print(temp_string);
}

/**************************************************************************************************
** Send a uint8 as 8 binary digits
*/
void TERM_tx_bin8(uint8_t bin)
{
	uint8_t		i;

	for (i = 0; i < 8; i++)
	{
		if ((bin & (1<<7)) != 0)
			TERM_tx_char('1');
		else
			TERM_tx_char('0');
		bin <<= 1;
	}
}
