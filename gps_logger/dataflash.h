/*
 * dataflash.h
 * Project:	Micro Phocus
 * Author:	Paul Qureshi
 * Created: 18/11/2011 15:38:53
 */


#ifndef DATAFLASH_H_
#define DATAFLASH_H_



#define DF_PAGE_SIZE				264
#define DF_NUM_PAGES				32768
#define DF_BUF1						false
#define DF_BUF2						true

#define DF_PORT						PORTB
#define DF_RES_PIN_bm				PIN6_bm
#define DF_CS_PIN_bm				PIN7_bm

#define DF_EN						DF_PORT.OUTCLR = DF_CS_PIN_bm
#define DF_DIS						DF_PORT.OUTSET = DF_CS_PIN_bm



typedef	struct
{
	uint8_t	manufacturer_id;
	uint8_t	device_id_1;
	uint8_t	device_id_2;
} DF_DEVID_t;



// Dataflash commands
#define DF_CMD_RD_DEVID				0x9F		// Manufacturer and Device ID Information
#define DF_CMD_STATUS				0xD7		// Read status register
#define DF_CMD_PAGE_ERASE			0x81		// Erase entire page
#define DF_CMD_SECTOR_ERASE			0x7C		// Erase entire sector
#define DF_CMD_CHIP_ERASE			0xC7		// Erase entire chip
#define DF_CMD_ARRAY_READ			0x0B		// continuous array read
#define DF_CMD_BUF1_WRITE			0x84
#define DF_CMD_BUF2_WRITE			0x87
#define DF_CMD_WRITE_BUF1_W_ERASE	0x83		// write buffer 1 to flash memory with erase
#define DF_CMD_WRITE_BUF2_W_ERASE	0x86		// write buffer 2 to flash memory with erase
#define DF_CMD_WRITE_BUF1_NO_ERASE	0x88		// write buffer 1 to flash memory without erase
#define DF_CMD_WRITE_BUF2_NO_ERASE	0x89		// write buffer 2 to flash memory without erase
#define DF_CMD_READ_PAGE_TO_BUF1	0x53
#define DF_CMD_READ_PAGE_TO_BUF2	0x55
#define DF_CMD_POWER_DOWN			0xB9
#define DF_CMD_RESUME				0xAB
#define DF_CMD_READ_SECURITY_REG	0x77



#define DF_end_array_read()			DF_DIS
#define DF_end_buffer_write()		DF_DIS


extern void		DF_init(void);
extern void		DF_wait_for_ready(void);
extern bool		DF_ready(void);
extern void		DF_erase_page(uint16_t page);
extern void		DF_erase_sector(uint8_t sector);
extern void		DF_start_array_read(uint16_t page, uint16_t byte);
extern void		DF_start_buffer_write(bool buffer2, uint16_t byte);
extern void		DF_start_buffer_write_no_busy_check(bool buffer2, uint16_t byte);
extern void		DF_write_buffer(bool buffer2, bool erase, uint16_t page);
extern void		DF_print_page(uint16_t page);
extern void		DF_erase_entire_memory(void);
extern void		DF_spi_buffer(void *buffer, uint16_t buffer_size);



#endif /* DATAFLASH_H_ */