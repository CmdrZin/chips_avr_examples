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
* main.c
*
* Created: 4/14/2017	0.01	ndp
*  Author: Chip
* Revised: 8/8/2017		0.10	ndp	Change process to simple.
* Revised: 4/23/2018	0.20	ndp Hack for RC Decoder Slave.
*
* Demo code for RC Decoder Slave project.
* Target: ATmega88A (be sure to set Project > Properties > Device to this AVR chip)
* NOTE: Uncheck the CKDIV8 fuse to use an 8 MHz CPU clock.
*
* This code demonstrates a simple RC Servo signal decoder.
* It will also allow the Master to set the PWM for the Steering and Throttle.
*
* Write Commands from the Master
* Commands:
*	SDA_W 00 00	- turn the LED OFF
*	SDA_W 00 01	- turn the LED ON
*	SDA_W 03 TT	- Set Throttle to TT (0x00 Full reverse..0x80 Stop..0xFF Full forward)
*	SDA_W 04 SS	- Set Steering to SS (0x00 Full Left .. 0x80 Center .. 0xFF Full Right)
* where SDA_W is the I2C address in Write Mode. [ i.e. (SLAVE_ADRS<<1)|0 ]
*
* Read Commands from the Master
* Use SDA_W REG to set register index.
* Commands:
*	SDA_R INDEX INDEX+1 ...
* where SDA_R is the I2C address in Read Mode. [ i.e. (SLAVE_ADRS<<1)|1 ]
*
 * txBuf[0] = 0x00 		always
 * txBuf[1] = ADCH		Left shifted..upper 8 bits
 * txBuf[2] = ADCL		Lower 2 bits
 * txBuf[3] = TROTTLE	One byte
 * txBuf[4] = STEERING	One byte
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
#include "twiSlave.h"
#include "mod_led.h"
#include "mod_adc.h"
#include "mod_rcDecode.h"


void test01(int c);

#define SLAVE_ADRS	0x5C

typedef enum {IDLE, READ_DATA} RX_STATES;

int main(void)
{
	uint8_t data;
	RX_STATES state = IDLE;
	long timeWait = 0;

	st_init_tmr0();
	mod_led_init();
	twiSlaveInit( SLAVE_ADRS );	// Initialize USI hardware for I2C Slave operation.
	mod_adc_init();
	mod_rcDecode_init();

	// TEST DATA LOAD
	twiSetRegister( 1, 0x11 );			// ADCH
	twiSetRegister( 2, 0x22 );			// ADCL
	twiSetRegister( 3, 0x33 );			// THROTTLE
	twiSetRegister( 4, 0x44 );			// STEERING

	sei();							// Enable interrupts.
	
	test01(6);
	
	twiSlaveEnable();			// Enable the USI interface to receive data.

	// A simple loop to check for I2C Commands.
	// Format is SDA_W REG D0	Only REG = 1 (LED) supported.
	// Data read is handled in the twi driver.
	while(1)
	{
		if( timeWait < st_millis() ) {
			mod_adc_getAndTrigger();				// update temperature registers.
			timeWait = st_millis() + 1000;			// set to 1 sec wait.
		}

		if( twiDataInReceiveBuffer() )
		{
			data = twiReceiveByte();

			switch(state) {
				case IDLE:
					twiSetRegisterIndex(data);		// Set to the last data byte of a SDA_W command.
					// TODO This needs to change to the first byte after a new SDA_W operation. Need a flag or getStatus().

					// TODO This will need to change to allow writing to various control registers.
					// Only Reg 00 is supported for data write. Any can be used for data read.
					if(data == 0) {
						state = READ_DATA;
					}
					break;
					
				case READ_DATA:
					// Change LED state
					// If the data is 00, then turn OFF the LED. Turn it ON for any non-zero value.
					// NOTE: LED hardware is wired 'Active LOW'.
					if( data == 0) {
						mod_led_off();				// Turn LED OFF.
					} else {
						mod_led_on();				// Turn LED ON.
					}
					state = IDLE;
					break;

				default:
					state = IDLE;
					break;
			}
		}
	}
}

void test01(int cnt)
{
	long nextTime = st_millis() + 500;
	uint8_t tog = 0;
	int count = 0;
	
	while(count < cnt) {
		if(nextTime < st_millis()) {
			nextTime = nextTime + 500;
			++count;				// only inc if timeout occurs.
			if(tog == 0) {
				tog = 1;
				mod_led_on();
			} else {
				tog = 0;
				mod_led_off();
			}
		}
	}
}
