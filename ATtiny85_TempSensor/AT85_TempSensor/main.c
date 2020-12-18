/*
* The MIT License (MIT)
*
* Copyright (c) 2016-2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
* main.c
*
* Created: 4/14/2017	0.01	ndp
*  Author: Chip
* Revised: 8/8/2017	0.10	ndp	Change process to simple.
*
* Demo code for Bootloader project.
* Target: ATtiny85 (by sure to set Project > Properties > Device to this AVR chip)
* NOTE: Uncheck the CKDIV8 fuse to use an 8 MHz CPU clock.
*
* This code demonstrates a simple I2C Slave to control an LED and read a counter.
*
* Write Commands from the Master
* Commands:
*	SDA_W 00	- turn the LED OFF
*	SDA_W 01	- turn the LED ON
* where SDA_W is the I2C address in Write Mode. [ i.e. (SLAVE_ADRS<<1)|0 ]
*
* Read Commands from the Master
* Commands:
*	SDA_R DH DL	- return Temperature. HighByte LowByte
* where SDA_R is the I2C address in Read Mode. [ i.e. (SLAVE_ADRS<<1)|1 ]
*
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "usiTwiSlave.h"
#include "mod_led.h"
#include "mod_adc.h"

#define SLAVE_ADRS	0x40

int main(void)
{
	uint8_t data;

	mod_led_init();
	usiTwiSlaveInit( SLAVE_ADRS );	// Initialize USI hardware for I2C Slave operation.
	mod_adc_init();

	sei();							// Enable interrupts.
	
	usitwiSlaveEnable();			// Enable the USI interface to receive data.

	// A simple loop to check for I2C Commands.
	// A state variable is needed to process multi-byte messages.
	// 01, 05 are Writes. 04 is a Read.
	while(1)
	{
		if( usiTwiDataInReceiveBuffer() )
		{
			data = usiTwiReceiveByte();
			// Change LED state
			// If the data is 00, then turn OFF the LED. Turn it ON for any non-zero value.
			// NOTE: LED hardware is wired 'Active LOW'.
			if( data == 0)
			{
				mod_led_off();				// Turn LED OFF.
			}
			else
			{
				mod_led_on();				// Turn LED ON.
			}
		}
	}
}
