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
 * adc_support.c
 *
 * Created: 9/13/2016	0.01	ndp
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "adc_support.h"
#include "sysTimer.h"


uint8_t as_startWaitRead( uint8_t chan );

uint8_t as_data[8];		// last data read
uint8_t as_channel;

/*
 * Vref: VCC
 * Clock: 20MHz -> CK/128 = 156kHz
 * Data: Left shift
 */
void adc_support_init()
{
	// Set up ADC	
	ADMUX = (0<<REFS1) | (1<<REFS0) | (1<<ADLAR);
	DIDR0 = 0x3F;
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

	as_channel = 0;
}

/*
 * Read one channel every 1ms. 8ms to read all eight channels.
 * Process - Called continuously (self timed 1ms steps)
 * 0. Read as_channel
 * 1. inc as_channel and reset to 0 if == 8
 */
void adc_support_service()
{
	static uint32_t lastTime = 0;
	uint32_t currentTime;
	
	currentTime = st_millis();
	
	if( currentTime > lastTime )
	{
		lastTime = currentTime;
		
		as_data[as_channel] = as_startWaitRead(as_channel);		// Trigger-Wait-Read

		if(++as_channel >= 8) {
			as_channel = 0;
		}
	}
}

/*
 * Return the latest ADC reading for channel.
 */
uint8_t adc_support_readChan( uint8_t chan )
{
	return as_data[chan];
}

/*
 * Start ADC
 * Wait for conversion
 * Read ADC
 * ~15-20us delay
 */
uint8_t as_startWaitRead( uint8_t chan )
{
	ADMUX &= 0xE0;					// clear last channel.

	ADMUX |= chan;
	
	ADCSRA |= (1<<ADSC);
	
	while( ADCSRA & (1<<ADSC) );	// wait for conversion to complete.
	
	return ADCH;
}
