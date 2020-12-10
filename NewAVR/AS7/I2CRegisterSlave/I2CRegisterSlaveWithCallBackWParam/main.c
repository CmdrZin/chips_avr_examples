/*
* The MIT License
*
* Copyright 2020 CmdrZin.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* I2C Register Slave
*
* Created: 11/09/2020 10:46:43 AM
* Author : Chip
* Revised: 11/15/2020
*
* This project uses a register instead of a FIFO for receiving and sending data.
* This approach makes it look more like a hardware device.
* The project is configured to use the Callback feature in twiRegSlave.
*	twiRegSlave: Data Register based Slave I2C interface with Callback enabled.
* This example uses callback functions. Ones that can be passed a parameter.
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTime.h"
#include "mod_led.h"

// Only enable ONE of these.
#include "twiRegSlaveCBwParam.h"

void RegisterOneCallBack_w(uint8_t n);

#define LED_DELAY		1000UL		// N * 1ms
#define TWI_DELAY		1000UL
#define SLAVE_ADRS		0x56

volatile uint8_t rxRegister[16];
volatile uint8_t txRegister[16];

int main(void)
{
	uint32_t ledTime = 0UL;
	uint32_t twiTime = 0UL;

	int loopCount = 0;
	
	init_time();		// set up clock and timers
	mod_led_init();		// set up LED pin
	twiRegSlaveInit(SLAVE_ADRS, rxRegister, sizeof(rxRegister), txRegister, sizeof(txRegister));

	sei();				// enable global interrupts
	
	// Start up Blink 3 times.
	while(loopCount < 7) {
		if( st_millis() > ledTime ) {
			ledTime += LED_DELAY;
			if(loopCount % 2) {
				mod_led_on();
				} else {
				mod_led_off();
			}
			++loopCount;
		}
	}
	
	while (1)
	{
		// disable when testing with Arduino_I2C_Master which also blinks the LED.
		#if 0
		// Burst blink the LED every second
		if( st_millis() > ledTime ) {
			ledTime += LED_DELAY;
			mod_led_toggle(250);
		}
		#endif
		// Demo TWI I2C
		if( st_millis() > twiTime ) {
			twiTime += TWI_DELAY;
			// Put some values into the registers to send out.
			// TODO: Replace this with ADC read of temperature.
			twiSetRegister(1, loopCount>>8);
			twiSetRegister(2, loopCount++);
		}

		// Allow the Master to control the LED by writing to register 1.
		// A callback function now controls the LED.
		//		if( twiGetRegister(1) ) {
		//			mod_led_on();
		//		} else {
		//			mod_led_off();
		//		}
	}
}

// This functions is called when Register 1 is written to when using twiRegSlaveCBwParam.
void RegisterOneCallBack_w(uint8_t n) {
	if( n != 0 ) {
		mod_led_on();
		} else {
		mod_led_off();
	}
}
