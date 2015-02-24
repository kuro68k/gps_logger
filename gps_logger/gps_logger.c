/*
 * gps_logger.c
 *
 * Created: 18/11/2014 16:08:23
 *  Author: paul.qureshi
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <asf.h>

#include "global.h"
#include "version.h"
#include "hw.h"
#include "gps.h"
#include "terminal.h"
#include "fram.h"


int main(void)
{
	// start watchdog
	while (WDT.STATUS & WDT_SYNCBUSY_bm)
		;
	WDR();
	HW_CCPWrite(&WDT_CTRL, WDT_PER_8KCLK_gc | WDT_ENABLE_bm | WDT_CEN_bm);

	sysclk_init();

	// power down unneeded hardware
	SLEEP.CTRL	= SLEEP_SMODE_IDLE_gc | SLEEP_SEN_bm;
	PR.PRGEN	= PR_AES_bm | PR_EVSYS_bm | PR_DMA_bm;
	PR.PRPA		= PR_DAC_bm | PR_ADC_bm | PR_AC_bm;
	PR.PRPB		= PR_DAC_bm | PR_ADC_bm | PR_AC_bm;
	PR.PRPC		= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
	PR.PRPD		= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
	PR.PRPE		= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
	PR.PRPF		= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;

	// start interrupts
	PMIC.CTRL	= PMIC_RREN_bm | PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
	sei();
	
	HW_init();
	TERM_init();
	TERM_printf_P(PSTR("GPS Logger V%u.%u\r\n"), VERSION_MAJOR, VERSION_MINOR);
	TERM_print_P(PSTR("FW built: " __DATE__ " " __TIME__ "\r\n"));
	TERM_printf_P(PSTR("Last reset:\t%02X "), HW_last_reset_status);
	if (HW_last_reset_status & RST_SRF_bm)
		TERM_print_P(PSTR("SR "));
	if (HW_last_reset_status & RST_PDIRF_bm)
		TERM_print_P(PSTR("PDI "));
	if (HW_last_reset_status & RST_WDRF_bm)
		TERM_print_P(PSTR("WDR "));
	if (HW_last_reset_status & RST_BORF_bm)
		TERM_print_P(PSTR("BOR "));
	if (HW_last_reset_status & RST_EXTRF_bm)
		TERM_print_P(PSTR("EXT "));
	if (HW_last_reset_status & RST_SRF_bm)
		TERM_print_P(PSTR("POR"));
	TERM_newline();

	//FRAM_init();
	//udc_start();

	GPS_run();
}