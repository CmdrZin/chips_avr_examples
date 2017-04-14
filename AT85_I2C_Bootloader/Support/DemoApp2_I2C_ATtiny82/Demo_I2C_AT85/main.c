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
 *
 * Demo code for Bootloader project.
 * Target: ATtiny85 (by sure to set Project > Properties > Device to this AVR chip)
 * NOTE: Uncheck the CKDIV8 fuse to use an 8 MHz CPU clock.
 *
 * This code demonstrates a simple I2C Slave to control an LED and read a counter.
 *
 * Write Commands from the Master
 * Control Registers:
 *	Control		0x01			b7-b1: unused, b0: Counter Reset if 1.
 *	LED			0x05
 * Commands:
 *	SDA_W 01 01	- Reset counter
 *	SDA_W 05 00	- turn the LED OFF
 *	SDA_W 05 01	- turn the LED ON
 * where SDA_W is the I2C address in Write Mode. [ i.e. (SLAVE_ADRS<<1)|0 ]
 *
 * Read Commands from the Master
 * Control Registers:
 *	Counter		0x04
 * Commands:
 *	SDA_W 04 SDA_R <count>	- return count
 * where SDA_W is the I2C address in Write Mode. [ i.e. (SLAVE_ADRS<<1)|0 ]
 * where SDA_R is the I2C address in Read Mode. [ i.e. (SLAVE_ADRS<<1)|1 ]
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "usiTwiSlave.h"

#define SLAVE_ADRS	0x40

// LED hardware support
#define LED_DDR		DDRB			// Port direction bit. 0:Input 1:output
#define LED_PORT	PORTB			// Port being used.
#define LED_P		PB1				// Port bit being used.

typedef enum {PS_IDLE, PS_CMD01_0, PS_CMD05_0 } PS_STATE;

// Static variables
PS_STATE	pState;						// Process state.
uint8_t		mCounter;					// simple message counter.

uint8_t		controlReg;					// Control Register: b7-b1 unused, b0: Counter reset if 1

int main(void)
{
	uint8_t data;

	pState	= PS_IDLE;
	mCounter = 0;

	LED_DDR |= (1<<LED_P);			// Set LED pin as an output.
	
	usiTwiSlaveInit( SLAVE_ADRS );	// Initialize USI hardware for I2C Slave operation.
	
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
			switch (pState)
			{
				case PS_IDLE:
					// Process new message
					++mCounter;

					switch(data)
					{
						case 01:
							// Writing to the Control Register
							pState = PS_CMD01_0;	// next byte is Control byte
							break;

						case 04:
							// Reading counter
							usiTwiTransmitByte(mCounter);		// load up data for following read.
							break;

						case 05:
							// Writing to LED
							pState = PS_CMD05_0;	// next byte controls LED
							break;

						default:
							break;					// Ignore unknown command.
					}
					break;

				case PS_CMD01_0:
					// Process Control byte. b0=1 clears counter.
					if( (data & 0x01) == 0x01 )
					{
						mCounter = 0;
					}
					pState	= PS_IDLE;				// reset for next message
					break;

				case PS_CMD05_0:
					// Change LED state
					// If the data is 00, then turn OFF the LED. Turn it ON for any non-zero value.
					// NOTE: LED hardware is wired 'Active LOW'.
					if( data == 0)
					{
						LED_PORT |= (1<<LED_P);			// Turn LED OFF.
					}
					else
					{
						LED_PORT &= ~(1<<LED_P);		// Turn LED ON.
					}
					pState	= PS_IDLE;				// reset for next message
					break;

				default:
					pState	= PS_IDLE;				// ERROR, restore to know state
					break;
			}
		}
    }
}
