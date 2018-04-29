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
 * Created: 12/03/2016		0.01	ndp
 * Author: Chip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"

#define SLOW_TIC		10			// 100us * N for the slow tic @ 1ms

uint8_t	st_cnt_1ms;					// secondary timer counter.

/*
 * Set up Timer0 to generate System Time Tic for 100us (10kHz) using 8MHz CPU clock
 * Call this once after RESET.
 *
 * Modifies: OCR0A, TCCR0A, TIMSK0, TCCR0B, and GPIOR0
 *
 * input reg:	none
 * output reg:	none
 * resources:	R16
 *
 * NOTE: 1ms
 */
void st_init_tmr0()
{
	OCR0A = 99;					// 8,000,000 / 800 = 10,000 : 8 * (1 + OCR0A) : 1 * (1+99)

	TCCR0A = (1<<WGM01);

	TIMSK = (1<<OCIE0A);		// enable counter 0 OCO interrupt

	TCCR0B =  0b010;			// CPU div 8

	st_cnt_1ms = SLOW_TIC;

	GPIOR0 = 0;					// clear all tic flags

	return;
}

/*
 * Timer0 CTC (compare) interrupt service.
 * Called each 100us
 *
 * input reg:	none
 * output reg:	none
 * resources:	GPIOR0.GPIR00:7
 * 				SRAM	1 byte
 *				Stack:3
 *
 */
ISR(TIMER0_COMPA_vect)
{
	// tic100us flags
	GPIOR0 |= (1 << 0);
	GPIOR0 |= (1 << 1);
	GPIOR0 |= (1 << 2);
	GPIOR0 |= (1 << 3);

	if( --st_cnt_1ms == 0 )
	{
		GPIOR0 |= (1 << 4);
		GPIOR0 |= (1 << 5);
		GPIOR0 |= (1 << 6);
		GPIOR0 |= (1 << 7);
		st_cnt_1ms = SLOW_TIC;
	}
}
