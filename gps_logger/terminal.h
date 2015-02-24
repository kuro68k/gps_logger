/*
 * terminal.h
 *
 * Created: 19/11/2014 11:42:27
 *  Author: paul.qureshi
 */ 


#ifndef TERMINAL_H_
#define TERMINAL_H_


#define	TERM_USART			USARTD0
#define TERM_PORT			PORTD
#define TERM_RX_PIN_bm		PIN2_bm
#define	TERM_TX_PIN_bm		PIN3_bm


// macros
#define TERM_newline()	TERM_tx_char('\r'); TERM_tx_char('\n');


// public variables/functions
extern void TERM_init(void);
extern void TERM_tx_char(char c);
extern void TERM_print_P(const __flash char *string);
extern void TERM_print(char *string);
extern void TERM_printf_P(PGM_P format, ...);
extern void TERM_tx_bin8(uint8_t bin);




#endif /* TERMINAL_H_ */