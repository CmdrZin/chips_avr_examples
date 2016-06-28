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
 * 3pi_test01 - an application for the Pololu 3pi Robot
 *
 *
 * Created: 6/26/2016 2:12:25 PM
 *  Author: Chip
 */

#include <avr/io.h>

#include "dev_leds.h"

uint16_t count = 0;

int main()
{
	dev_leds_init();
	
	// Initial tests
	while( count == 0 )
	{
		++count;
		
		if( count % 30000 == 0 )
		{
			dev_red_led(true);
		}
		if( count % 30000 == 10000 )
		{
			dev_grn_led(true);
		}
		if( count % 30000 == 15000 )
		{
			dev_red_led(false);
		}
		if( count % 30000 == 25000 )
		{
			dev_grn_led(false);
		}
	}






}
