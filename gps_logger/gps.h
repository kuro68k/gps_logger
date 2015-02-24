/*
 * gps.h
 *
 * Created: 18/11/2014 16:09:30
 *  Author: paul.qureshi
 */ 


#ifndef GPS_H_
#define GPS_H_



typedef struct
{
	uint8_t	d, m, y;		//	3		3
	uint8_t	hr, min, sec;	//	3		6
	float	kph;			//	4		10
	int16_t	lat_left;		//	2		12
	int16_t	lat_right;		//	2		14
	int16_t	lon_left;		//	2		16
	int16_t	lon_right;		//	2		18
} GPS_DATA_t;


typedef struct
{
	uint32_t	counter;
	uint32_t	crc32;
} GPS_PAGE_HEADER_t;


#define BUFFERED_RECORDS	28


#define GPS_PORT			PORTC
#define GPS_RX_PIN_bm		PIN2_bm
#define GPS_TX_PIN_bm		PIN3_bm

#define GPS_USART			USARTC0


extern void GPS_dump(void);
extern void GPS_run(void);



#endif /* GPS_H_ */