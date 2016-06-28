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
 *
 * dev_leds.c
 *
 * Created: 6/26/2016		ndp		0.01
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "dev_leds.h"

#define DEV_RED_LED_DDR		DDRD
#define DEV_RED_LED_PORT	PORTD
#define DEV_RED_LED_OUT		PORTD1

#define DEV_GRN_LED_DDR		DDRD
#define DEV_GRN_LED_PORT	PORTD
#define DEV_GRN_LED_OUT		PORTD7


void dev_red_led( bool state );
void dev_grn_led( bool state );

/*
 * Initialize LED IO pins to be OUTPUTS.
 */
void dev_leds_init()
{
	DEV_RED_LED_DDR |= (1<<DEV_RED_LED_OUT);			// set HIGH for output
	DEV_GRN_LED_DDR |= (1<<DEV_GRN_LED_OUT);			// set HIGH for output
	return;
}

/*
 * Turn ON or OFF Green LED
 */
void dev_red_led( bool state )
{
	if(state)
	{
		DEV_RED_LED_PORT |= (1<<DEV_RED_LED_OUT);		// set HIGH for ON
	}
	else
	{
		DEV_RED_LED_PORT &= ~(1<<DEV_RED_LED_OUT);		// set LOW for OFF
	}
	return;
}

/*
 * Turn ON or OFF Red LED
 */
void dev_grn_led( bool state )
{
	if(state)
	{
		DEV_GRN_LED_PORT |= (1<<DEV_GRN_LED_OUT);		// set HIGH for ON
	}
	else
	{
		DEV_GRN_LED_PORT &= ~(1<<DEV_GRN_LED_OUT);		// set LOW for OFF
	}
	return;
}
