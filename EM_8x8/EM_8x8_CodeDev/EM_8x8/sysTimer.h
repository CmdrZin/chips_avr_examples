/*
 * sysTimer.h
 *
 * Created: 5/19/2015 1:06:23 PM
 *  Author: Chip
 * revision: 8/1/2015	0.01	ndp
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

#include <avr/io.h>

// 1ms tic flags
#define DEV_1MS_TIC		0			// Device service tic
//#define				1
//#define				2
//#define				3
// 10ms tic flags
#define	DEV_10MS_TIC	4
//#define				5
//#define				6
//#define				7

void st_init_tmr0();

void st_init_tmr2();
uint8_t tmr2_getCount();
void tmr2_clrCount();
void st_tmr2_clr();


#endif /* SYSTIMER_H_ */