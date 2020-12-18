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
 * mod_adc.c
 *
 * Created: 7/30/2017 10:58:18 AM
 *  Author: Chip
 */ 
#include <avr/io.h>
#include <stdbool.h>

#include "mod_adc.h"
#include "sysTimer.h"


uint16_t mod_adc_tempurature;

/*
 * Set up for sampling at 1s rate.
 *
 * ADCMUX = 1111 when input = ADC4
 * 10bit right adjusted.
 * 1.1v Vref for Temperature
 * use clk/64 for 125kHz conversion rate.
 */
void mod_adc_init()
{
	ADMUX = (0<<REFS2)|(1<<REFS1)|(0<<REFS0)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0);		//Vref=1.1v, Right Shift, MUX=Temperature

	ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);							// Enable, Start, clk/64
}
