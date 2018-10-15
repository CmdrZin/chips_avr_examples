/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016-2018 Nels D. "Chip" Pearson (aka CmdrZin)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
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
 * Revised: 4/25/2018		0.10	ndp	Added changes for RC Servo. Driver is CUSTOM now.
 * Revised: 9/24/2018		0.11	ndp Adjust for ATmega88A
 * Revised: 10/14/2018		0.20	npd adjsut for this project
 *
 */ 
#include <avr/io.h>
#include <stdbool.h>

#include "mod_adc.h"
#include "sysTimer.h"
#include "mod_comms.h"


uint16_t mod_adc_tempurature;

/*
 * Set up for triggered sampling.
 *
 * ADCMUX = 1000 for temperature channel
 * 10bit right adjusted.
 * 1.1v Vref for Temperature
 * use clk/64 for 125kHz conversion rate with 8MHz clock.
 */
void mod_adc_init()
{
	ADMUX = (1<<REFS1)|(1<<REFS0)|(1<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0);		// Vref=1.1v, Right Shift, MUX=Temperature

	ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);							// Enable, Start, clk/64
}

// Get current ADC and trigger next conversion.
void mod_adc_getAndTrigger()
{
	if( !(ADCSRA & (1<<ADSC)) ) {
		mod_comm_setReg(2, ADCL);
		mod_comm_setReg(1, ADCH);
		ADCSRA |= (1<<ADSC);				// restart ADC
	}
}
