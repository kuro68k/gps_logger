/*
 * hw.h
 *
 * Created: 19/11/2014 16:48:26
 *  Author: paul.qureshi
 */ 


#ifndef HW_H_
#define HW_H_


/**************************************************************************************************
** Hardware
*/

#define SPI					SPIC
#define SPI_PORT			PORTC
#define SPI_SS_PIN_bm		PIN4_bm
#define SPI_MOSI_PIN_bm		PIN5_bm
#define SPI_MISO_PIN_bm		PIN6_bm
#define SPI_SCLK_PIN_bm		PIN7_bm



/**************************************************************************************************
** Macros
*/

// port settings
#define DISABLE_DIGITAL_INPUT(A)	A = (A & ~PORT_ISC_gm) | PORT_ISC_INPUT_DISABLE_gc
#define ENABLE_PULLUP(A)			A = (A & ~PORT_OPC_gm) | PORT_OPC_PULLUP_gc
#define ENABLE_PULLDOWN(A)			A = (A & ~PORT_OPC_gm) | PORT_OPC_PULLDOWN_gc

// wait for NVM to wake up for reading the signature row
#define HW_wait_for_sig_row()	do {} while ((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);



/**************************************************************************************************
** External variables and functions
*/

extern uint8_t	HW_last_reset_status;


extern void		HW_init(void);
extern void		HW_CCPWrite(volatile uint8_t *address, uint8_t value)		__attribute__((nonnull));
extern uint8_t	HW_spi(uint8_t tx);



#endif /* HW_H_ */