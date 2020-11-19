/* 
 * The MIT License
 *
 * Copyright 2020 CmdrZin.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * sysTime
 *
 * Created: 11/9/2020 1:36:30 PM
 *  Author: Chip
 *
 * Modified for TCAservo
 * NOTE: Bit definitions are in iom4809.h
 *
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTime20.h"

static uint32_t total20Milliseconds;

/* Initialize system clock and timers. ATmega4808/4809 */
void init_time20()
{
	// TCAservo.c sets up TCA0 for a 20ms period.
	// Enable TCA0 to generate an OVF interrupt.
	TCA0_SINGLE_INTCTRL |= TCA_SINGLE_OVF_bm;			// Set OVF interrupt flag ONLY
	// NOTE: Global interrupts are enabled with sei() after all interrupt
	// generating modules are initialized.
}

/* Return the total number of milliseconds since the project started. */
uint32_t st_millis20()
{
	uint32_t temp;				// make a holder for the counter.

	cli();						// Turn OFF interrupts to avoid corruption during a multi-byte read.
	temp = total20Milliseconds;	// get a copy while interrupts are disabled.
	sei();						// Turn interrupts back ON.

	return temp;				// return a 'clean' copy of the counter.
}

/* This interrupt service is called every 1 ms. */
ISR(TCA0_OVF_vect)
{
	++total20Milliseconds;

	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;	// clear interrupt flag.
}
