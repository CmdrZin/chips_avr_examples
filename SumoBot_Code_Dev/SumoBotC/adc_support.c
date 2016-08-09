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
 * Created: 7/25/2016 10:46:58 AM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "adc_support.h"
#include "sysTimer.h"

#define LINE_LED_DDR	DDRD
#define LINE_LED_PORT	PORTD
// Map to IO PORTD pin
#define LINE_LED_L	PD6
#define LINE_LED_C	PD3
#define LINE_LED_R	PD7

#define LINE_PT		6			// Line sensor ADC channel

#define	ADC_SERVICE_DELAY	100	// 10 for IR range, 1 for LINE.
#define	ADC_ERROR_RANGE		4

uint8_t as_startWaitRead( uint8_t chan );

uint8_t as_ir_range[6];		// IR range sensors.
uint8_t as_line[3];			// raw line sensors.
uint8_t as_state;
uint8_t as_delayCount;

/*
 * Vref: VCC
 * Clock: 20MHz -> CK/128 = 156kHz
 * Data: Left shift
 */
void adc_support_init()
{
	LINE_LED_DDR |= (1<<LINE_LED_L) | (1<<LINE_LED_C) | (1<<LINE_LED_R);	// set as OUTPUT
	LINE_LED_PORT |= (1<<LINE_LED_L) | (1<<LINE_LED_C) | (1<<LINE_LED_R);	// turn OFF LEDs
	
	// Set up ADC	
	ADMUX = (0<<REFS1) | (1<<REFS0) | (1<<ADLAR);
	DIDR0 = 0x3F;
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	
	as_state = 0;
	as_delayCount = ADC_SERVICE_DELAY;
}

/*
 * Process (1ms steps)
 * 0. Read FR
 * 1. Read FL and Set Line LED ON
 * 2. Read PT, Turn LED OFF, Turn next LED ON
 * 3. Repeat 2 for all 3 Line sensors.
 * TODO: Need to measure ambient then subtract it from actual. <<<------TODO
 */
void adc_support_service()
{
	if( GPIOR0 & (1<<ADC_1MS_TIC) )
	{
		GPIOR0 &= ~(1<<ADC_1MS_TIC);
		
		switch( as_state )
		{
			case 0:
				as_ir_range[IR_PT_FR] = as_startWaitRead(IR_PT_FR);	// Trigger-Wait-Read IR_PT_FR
				break;

			case 1:
				as_ir_range[IR_PT_FL] = as_startWaitRead(IR_PT_FL);	// Trigger-Wait-Read IR_PT_FL

				LINE_LED_PORT &= ~(1<<LINE_LED_L);		// LINE_LED_L ON
				break;

			case 2:
				as_line[LINE_L] = as_startWaitRead(LINE_PT);	// Trigger-Wait-Read for LINE_L
				LINE_LED_PORT |= (1<<LINE_LED_L);		// LINE_LED_L OFF
				LINE_LED_PORT &= ~(1<<LINE_LED_C);		// LINE_LED_C ON
				break;
			
			case 3:
				as_line[LINE_C] = as_startWaitRead(LINE_PT);	// Trigger-Wait-Read for LINE_C
				LINE_LED_PORT |= (1<<LINE_LED_C);		// LINE_LED_C OFF
				LINE_LED_PORT &= ~(1<<LINE_LED_R);		// LINE_LED_R ON
				break;

			case 4:
				as_line[LINE_R] = as_startWaitRead(LINE_PT);	// Trigger-Wait-Read for LINE_R
				LINE_LED_PORT |= (1<<LINE_LED_R);		// LINE_LED_R OFF
				break;

			default:
				as_state = 0;
				break;
		}
		if(++as_state > 4) { as_state = 0; }
	} // end of switch()
}

/*
 * Return the latest IR Range reading.
 */
uint8_t adc_support_readChan( uint8_t chan )
{
	return as_ir_range[chan];
}

/*
 * Return the latest Line reading.
 */
uint8_t adc_support_readLine( uint8_t sensor )
{
	return as_line[sensor];
}

/*
 * Start ADC
 * Wait for conversion
 * Read into array
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
