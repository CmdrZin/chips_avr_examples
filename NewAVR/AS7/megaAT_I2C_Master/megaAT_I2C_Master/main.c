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
* TWI I2C Master
*
* Created: 12/09/2020 10:46:43 AM
* Author : Chip
*	
* This project uses a FIFO for receiving and sending data.
*	twiMaster: I2C Master interface.
*
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTime.h"
#include "mod_led.h"
#include "twiMaster.h"

#define LED_DELAY		500UL		// N * 1ms
#define TWI_DELAY		200UL
#define SLAVE_ADRS		0x08

uint8_t rxRegister[16];
uint8_t txRegister[16];

int main(void)
{
	uint32_t ledTime = 0UL;
	uint32_t twiTime = 0UL;

	int loopCount = 0;
	
	init_time();		// set up clock and timers
	mod_led_init();		// set up LED pin
	twiMasterInit();

	sei();				// enable global interrupts
 mod_led_toggle(1);
	
	// Start up Blink 3 times.
#if 1
	while(loopCount < 7) {
		if( st_millis() > ledTime ) {
			ledTime = st_millis() + LED_DELAY;
			if(loopCount % 2) {
				mod_led_on();
			} else {
				mod_led_off();
			}
			++loopCount;
		}
	}
#endif

	while (1)
	{
#if 1
		// Burst blink the LED every second
		if( st_millis() > ledTime ) {
			ledTime = st_millis() + LED_DELAY;
			mod_led_toggle(250);
		}
#endif
		// Demo TWI I2C
		if( st_millis() > twiTime ) {
			twiTime = st_millis() + TWI_DELAY;

			twiRead(SLAVE_ADRS, rxRegister, 6);
			while(twiIsBusy());								// Wait for Read to complete.
			// Copy last Read to txRegister to send out.
			for(int i=0; i<6; i++) {
				txRegister[i] = rxRegister[i];
			}
			twiWrite(SLAVE_ADRS, txRegister, 6);
		}
	}
}
