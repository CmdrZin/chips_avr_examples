/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * Created: 5/19/2015	0.01	ndp
 * Author: Chip
 * revised: 9/09/2017	0.10	ndp replace GPIO tics with st_millis() function.
 *
 * ALL timers based on 20MHz CPU clock.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"

static volatile uint32_t	totalMilliseconds;		// Total millisecond since POR.
													// Use static to restrict scope to this module.

/*
 * Set up Timer0 to generate System Time Tic for 1 ms using 20MHz CPU clock
 * Call this once after RESET.
 *
 * Modifies: OCR0A, TCCR0A, TIMSK0, TCCR0B
 * NOTE: 1ms
 */
void st_init_tmr0()
{
	OCR0A = 77;					// (20MHz) 2 * 256 * (1 + OCR0A) : 512 * (40*2) : 1024 * 40 * 2

	TCCR0A = (1<<WGM01);

	TIMSK0 = (1<<OCIE0A);		// enable counter 0 OCO interrupt

	TCCR0B =  0b100;			// CPU div 256

	totalMilliseconds = 0L;

	return;
}

/*
 * Return milliseconds since Power-On-Reset.
 * Even though volatile, four byte value must be protected from change.
 */
uint32_t st_millis()
{
	uint32_t temp;
	
	cli();
	temp = totalMilliseconds;
	sei();
	
	return temp;
}


/*
 * Timer0 CTC (compare) interrupt service.
 * Called each 1ms
 */
ISR(TIMER0_COMPA_vect)
{
	++totalMilliseconds;
}
