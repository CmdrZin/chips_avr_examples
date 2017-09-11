/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * SlaveADC.c
 *
 * Created: 9/07/2017	0.01	ndp
 * Author : Chip
 *
 * Target: ATmega164P, 20MHz
 *
 * This is the main() file for the SlaveADC board.
 * It provides 8 channels of 10 bit ADC, a status LED, and two five pin I/O ports.
 * Optional Audio and Serial hardware can be added.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "sysTimer.h"
#include "mod_led.h"
#include "adc_support.h"
#include "twiSlave.h"
#include "core_functions.h"

#define SLAVE_ADRS		0x32

void mTest01();
void mTest02();

/*
 * main()
 *
 * This is a sequential scheduler that services all available tasks
 * and then checks for a complete message and forwards it to the 
 * addressed task access process.
 *
 * Each task manages their own time slice.
 *
 */
int main(void)
{
	uint8_t	data;
	
	st_init_tmr0();
	mod_led_init();
	adc_support_init();
	twiSlaveInit(SLAVE_ADRS);
	
	sei();							// Enable global interrupts.

	twiSlaveEnable();
		
	while(1)
	{
		adc_support_service();					// sample a channel each ms.
		
		if( twiDataInReceiveBuffer() )
		{
			// NOTE: Master should wait at least 1ms after requesting data before reading it. No clock stretching.
			data = twiReceiveByte();
			
			switch(data)
			{
				// Read an ADC channel
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					twiClearOutput();
					twiTransmitByte(adc_support_readChan(data));
					break;
					
				// Read ALL channels
				case 8:
					twiClearOutput();
					for(int i=0; i<8; i++) {
						twiTransmitByte(adc_support_readChan(i));
					}
					break;
					
				case 0xF8:
					core_get_build_date();
					break;
					
				case 0xF9:
					core_get_build_time();
					break;
					
				case 0xFA:
					core_get_version();
					break;
					
				default:
					break;
			}
		}
		
//		mTest01();
		mTest02();
	}
}

/*
 * Simple LED test
 */
void mTest01()
{
	static uint32_t	lastTime = 0L;
	static bool	tFlag = true;

	if((st_millis() - lastTime) > 100) {
		lastTime = st_millis();
		if(tFlag) {
			mod_led_on();
			tFlag = false;
		} else {
			mod_led_off();
			tFlag = true;
		}
	}
}

/*
 * ADC Read Pot test
 * Light LED if any pot > 0x80;
 */
void mTest02()
{
	mod_led_off();
	for(int i=0; i<8; i++)
	{
		if(adc_support_readChan(i) > 0x80)
		{
			mod_led_on();
		}
	}
}
