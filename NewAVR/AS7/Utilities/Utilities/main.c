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
 * Utilities
 *
 * Created: 11/9/2020 10:46:43 AM
 * Author : Chip
 *
 * This project holds utility function files with simple test code in main.c
 *   sysTime: Clock and Timer support.
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTime.h"

void toggleLED();

int main(void)
{
	uint32_t lastTime = 0UL;
	
	// initialize test IO
	PORTC_DIR = 0xFF;	// all outputs
	
	init_time();		// set up clock and timers

	sei();				// enable global interrupts
	
    /* Replace with your application code */
    while (1) 
    {
		// Check every ms
		if( st_millis() != lastTime ) {
			lastTime = st_millis();
			if(is10msFlagSet(3)) {
				toggleLED();
			}
		}
    }
}

void toggleLED()
{
	static bool flag = false;
	
	if(flag) {
		// set LED ON
		PORTC_OUT = 0xFF;		// all lines HIGH
	} else {
		// set LED OFF
		PORTC_OUT = 0;			// all lines LOW
	}
	flag = !flag;
}
