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
 * mod_led_1.c
 *
 * Created: 5/18/2015 8:59:27 PM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "mod_led_1.h"

#define DEMO_BOARD	0					// Use for testing. Set to 0 for proto-board set up.

void mod_led_1_init()
{
	DEV_LED_DDR |= (1<<DEV_LED_OUT_PIN);			// set HIGH for output

#if DEMO_BOARD == 1
	// Extras for demo board use.
	DDRB |= (1<<PB0);					// use Port B B0 as output
	PORTB |= (1<<PB0);
#endif
	return;
}

/*
 * Turn LED OFF
 * CMD: F0 DEV 01
 */
void mod_led_1_off()
{
#if DEMO_BOARD == 0
	DEV_LED_PORT &= ~(1<<DEV_LED_OUT_PIN);			// set LOW
#else
	// Reverse logic for demo board use.
	DEV_LED_PORT |= (1<<DEV_LED_OUT_PIN);			// set HIGH
#endif
	return;
}

/*
 * Turn LED ON
 * CMD: F0 DEV 02
 */
void mod_led_1_on()
{
#if DEMO_BOARD == 0
	DEV_LED_PORT |= (1<<DEV_LED_OUT_PIN);			// set HIGH
#else
	// Reverse logic for demo board use.
	DEV_LED_PORT &= ~(1<<DEV_LED_OUT_PIN);			// set LOW
#endif
	return;
}
