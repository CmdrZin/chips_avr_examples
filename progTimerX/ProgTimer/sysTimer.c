/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * sysTimer.c
 *
 * System Timer Utility
 *
 * Created: 5/19/2015		0.01	ndp
 * Author: Chip
 * revised: 01/31/2017		0.02	ndp		to use 1 MHz clock
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <time.h>

#include "sysTimer.h"
#include "io_led_button.h"			// DEBUG ONLY

#define SLOW_TIC		10			// 1ms * N for the slow tic

uint8_t	st_cnt_10ms;				// secondary timer counter.

time_t	st_localTime;

/*
 * Set up Timer0 to generate System Time Tic for 1 ms using 1 MHz CPU clock.
 * Call this once after RESET.
 */
void st_init_tmr0()
{
	OCR0A = 124;				// 1ms = 1MHz / 8 * (1 + OCR0A) : 8 * (1+124) : 1MHz / 1000 = 1ms
	TCCR0A = (1<<WGM01);		// CTC w/ TOP=OCRA
	TIMSK0 = (1<<OCIE0A);		// enable counter 0 OCO interrupt
	TCCR0B =  0b010;			// CPU div 8

	st_cnt_10ms = SLOW_TIC;
	GPIOR0 = 0;					// clear all tic flags

	return;
}

/*
 * Set up Timer2 to generate Wake-Up after 249.86ms using 1 MHz CPU clock.
 * Call this once after RESET.
 */
void st_init_tmr2()
{
#if 1
	ASSR = (1<<AS2);			// use external 32kHz xtal

	OCR2A = 255;				// 8s = 32.768kHz / 1024 * (1 + OCR0A) :1024 * (1+255) : 32768 / 262144 = 0.125 Hz = 8s
	while(ASSR & (1<<OCR2AUB));	// wait for write to complete.

	TCCR2A = (1<<WGM01);		// CTC w/ TOP=OCRA
	while(ASSR & (1<<TCR2AUB));	// wait for write to complete.

	TCCR2B =  0b0111;			// CPU div 1024
	while(ASSR & (1<<TCR2BUB));	// wait for write to complete.

	TIMSK2 = (1<<OCIE2A);		// enable counter 2 OCMA interrupt
#else
	OCR2A = 247;				// 250ms = 1MHz / 1024 * (1 + OCR0A) :1024 * (1+243) : 1MHz / 249856 = 4.0023 Hz = 249.86ms < 0.06% error
	TCCR2A = (1<<WGM01);		// CTC w/ TOP=OCRA
	TIMSK2 = (1<<OCIE2A);		// enable counter 2 OCMA interrupt
	TCCR2B =  0b111;			// CPU div 1024
#endif

	return;
}

void st_setLocalTime(time_t val)
{
	st_localTime = val;
}

time_t st_getLocalTime()
{
	return st_localTime;
}

/*
 * Get local time as IsoAscii
 */
char* st_getLocalAscii()
{
	const time_t	event = st_localTime;
	const struct tm* timeStrut = localtime(&event);

	return isotime(timeStrut);
}


/*
 * Timer0 CTC (compare) interrupt service.
 * Called each 1ms
 *
 * resources:	GPIOR0.GPIR00:7
 * 				SRAM	1 byte
 *
 */
ISR(TIMER0_COMPA_vect)
{
	// tic1ms flags
	GPIOR0 |= (1 << 0);
	GPIOR0 |= (1 << 1);
	GPIOR0 |= (1 << 2);
	GPIOR0 |= (1 << 3);

	if( --st_cnt_10ms == 0 )
	{
		GPIOR0 |= (1 << 4);
		GPIOR0 |= (1 << 5);
		GPIOR0 |= (1 << 6);
		GPIOR0 |= (1 << 7);
		st_cnt_10ms = SLOW_TIC;
	}
}

/*
 * Timer2 CTC (compare) interrupt service.
 * Called once each 8s with external 32kHz clock.
 *
 * Update Local clock.
 */
ISR(TIMER2_COMPA_vect)
{
	st_localTime +=8;
}
