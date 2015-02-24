/*
 * fram.h
 *
 * Created: 25/11/2014 14:11:14
 *  Author: paul.qureshi
 */ 


#ifndef FRAM_H_
#define FRAM_H_


/**************************************************************************************************
** Hardware
*/

#define FRAM_PORT				PORTE
#define FRAM_CS_PIN_bm			PIN0_bm
#define	FRAM_WP_PIN_bm			PIN1_bm
#define FRAM_HOLD_PIN_bm		PIN2_bm

#define FRAM_EN					do { FRAM_PORT.OUTCLR = FRAM_CS_PIN_bm; NOP(); } while(0);
#define FRAM_DIS				do { FRAM_PORT.OUTSET = FRAM_CS_PIN_bm; NOP(); } while(0);



/**************************************************************************************************
** SPI interface
*/

#define	FRAM_STATUS_WPEN_bm		(1<<7)
#define FRAM_STATUS_BP1_bm		(1<<3)
#define FRAM_STATUS_BP0_bm		(1<<2)
#define FRAM_STATUS_WEL_bm		(1<<1)
#define FRAM_STATUS_ZERO_bm		(1<<0)


#define FRAM_CMD_WREN			0b0110		// set write enable latch
#define FRAM_CMD_WRDI			0b0100		// reset write enable latch
#define FRAM_CMD_RDSR			0b0101		// read status register
#define FRAM_CMD_WRSR			0b0001		// write status register
#define FRAM_CMD_READ			0b0011		// read memory
#define FRAM_CMD_WRITE			0b0010		// write memory



/**************************************************************************************************
** Mapping and atomic storage
*/

#define FRAM_MAP_PAGE_ADDRESS	0x0000
#define FRAM_MAP_ATOMIC_META	0x0100
#define FRAM_MAP_ATOMIC_DATA	0x0108

#define FRAM_ATOMIC_MARKER		0xA7031C42

typedef struct
{
	uint32_t	marker;
	uint32_t	crc32;
} FRAM_ATOMIC_METADATA_t;



/**************************************************************************************************
** External variables and functions
*/

extern void FRAM_init(void);
extern void FRAM_write(uint16_t address, const void *buffer, uint16_t buffer_size);
extern void FRAM_read(uint16_t address, void *buffer, uint16_t buffer_size);
extern void FRAM_atomic_write(const void *buffer, uint16_t buffer_size);
extern bool FRAM_atomic_check(uint16_t buffer_size);
extern void FRAM_atomic_clear(void);



#endif /* FRAM_H_ */