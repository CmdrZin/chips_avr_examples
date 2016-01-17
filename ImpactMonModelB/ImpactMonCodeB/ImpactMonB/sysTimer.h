/*
 * sysTimer.h
 *
 * Created: 5/19/2015 1:06:23 PM
 *  Author: Chip
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

// 1ms tic flags
#define DEV_1MS_TIC		0			// Device service tic
#define	MCO_1MS_TIC		1			// Measurement sample tic
//#define				GPIOR02
//#define				GPIOR03
// 100ms tic flags
#define	DEV_100MS_TIC	4
//#define				GPIOR05
//#define				GPIOR06
//#define				GPIOR07

void st_init_tmr0();


#endif /* SYSTIMER_H_ */