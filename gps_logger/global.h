/*
 * global.h
 *
 * Created: 18/11/2014 16:10:10
 *  Author: paul.qureshi
 */ 


#ifndef GLOBAL_H_
#define GLOBAL_H_


#include <stdbool.h>


#define NOP()	__asm__ __volatile__("nop")
#define	WDR()	__asm__ __volatile__("wdr")



#endif /* GLOBAL_H_ */