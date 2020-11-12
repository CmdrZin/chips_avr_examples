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
*	mod_led: Single LED control.
*
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTime.h"
#include "mod_led.h"

#define LED_DELAY	500UL

int main(void)
{
	uint32_t nextTime = 0UL;
	uint8_t loopCount = 0;
	
	init_time();		// set up clock and timers
	mod_led_init();		// set up LED pin

	sei();				// enable global interrupts
	
	// Start up Blink 3 times.
	while(loopCount < 7) {
		if( st_millis() > nextTime ) {
			nextTime += LED_DELAY;
			if(loopCount % 2) {
				mod_led_on();
			} else {
				mod_led_off();
			}
			++loopCount;
		}
	}
	
	/* Replace with your application code */
	while (1)
	{
		// Check every ms
		if( st_millis() > nextTime ) {
			nextTime += LED_DELAY;
			mod_led_toggle(250);
		}
	}
}
