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
 * Time and Clock utilities for the ATmega4808/4809
 * NOTE: Bit definitions are in iom4809.h
 *
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTime.h"

static uint32_t totalMilliseconds;

static uint8_t st_10msFlags;

/* Initialize system clock and timers. ATmega4808/4809 */
void init_time()
{
	// Initialize MCU clock for 20 MHz
	CPU_CCP = CCP_IOREG_gc;		// unlock Change Protected Registers
	CLKCTRL_MCLKCTRLB = 0;		// Clear PEN bit to disable prescaler

	// Initialize TCA0 to generate an OVF interrupt every 1 ms.
	TCA0_SINGLE_PER = 0x4E1F;							// 20,000 - 1
	TCA0_SINGLE_CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;    // WGMODE[2:0] = 000
	TCA0_SINGLE_CTRLECLR = TCA_SINGLE_DIR_bm;			// Set DIR to up
	TCA0_SINGLE_INTCTRL = TCA_SINGLE_OVF_bm;			// Set OVF interrupt flag
	TCA0_SINGLE_CTRLA = TCA_SINGLE_ENABLE_bm;			// turn ON the timer. CLKSEL[2:0]=000 (Clock/1)
	// NOTE: Global interrupts are enabled with sei() after all interrupt
	// generating modules are initialized.
	
	st_10msFlags = 0;
}

/* Return the total number of milliseconds since the project started. */
uint32_t st_millis()
{
	uint32_t temp;				// make a holder for the counter.

	cli();						// Turn OFF interrupts to avoid corruption during a multi-byte read.
	temp = totalMilliseconds;	// get a copy while interrupts are disabled.
	sei();						// Turn interrupts back ON.

	return temp;				// return a 'clean' copy of the counter.
}

/* flag 0:7 */
bool is10msFlagSet( uint8_t flag )
{
	uint8_t mask = 0x01;
	bool result = false;

	// Position the mask bit to the flag bit to test
	mask <<= flag;
	// Test if the flag bit is set
	if( st_10msFlags & mask )
	{
		result = true;
		st_10msFlags &= ~mask;		// clear flag
	}

	return result;
}

/* This interrupt service is called every 1 ms. */
ISR(TCA0_OVF_vect)
{
	static uint8_t counter = 0;

	++totalMilliseconds;

	if(++counter >= 10) {
		st_10msFlags = 0xFF;
		counter = 0;
	}

	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;	// clear interrupt flag.
}
