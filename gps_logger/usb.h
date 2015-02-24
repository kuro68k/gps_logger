/*
 * usb.h
 *
 * Created: 26/11/2014 16:33:49
 *  Author: paul.qureshi
 */ 


#ifndef USB_H_
#define USB_H_


#include "udi_cdc.h"


#define USB_tx_char(c)		udi_cdc_putc(c)
#define USB_newline()		USB_tx_char('\r'); USB_tx_char('\n');
#define LE_CHR(a,b,c,d)		( ((uint32_t)(a)<<24) | ((uint32_t)(b)<<16) | ((c)<<8) | (d) )


extern void USB_print_P(const __flash char *string);
extern void USB_print(char *string);
extern void USB_printf_P(PGM_P format, ...);
extern void USB_tx_bin8(uint8_t bin);



#endif /* USB_H_ */