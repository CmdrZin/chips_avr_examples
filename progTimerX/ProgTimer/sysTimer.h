/*
 * sysTimer.h
 *
 * Created: 5/19/2015 1:06:23 PM
 *  Author: Chip
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

#include <time.h>

// 1ms tic flags
#define DEV_1MS_TIC		0			// Device service tic
//#define				GPIOR01
//#define				GPIOR02
//#define				GPIOR03
// 10ms tic flags
#define	DEV_10MS_TIC	GPIOR04
#define	AUDIO_10MS_TIC	GPIOR05
//#define				GPIOR06
//#define				GPIOR07

void st_init_tmr0();
void st_init_tmr2();

void st_setLocalTime(time_t val);
time_t st_getLocalTime();
char* st_getLocalAscii();


#endif /* SYSTIMER_H_ */