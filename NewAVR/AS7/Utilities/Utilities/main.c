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
* Utilities
*
* Created: 11/09/2020 10:46:43 AM
* Author : Chip
* Revised: 11/15/2020
*	
* This project holds utility function files with simple test code in main.c
*   sysTime:	 Clock and Timer support.
*	mod_led:	 Single LED control.
*	twiRegSlave: Data Register based Slave I2C interface.
*	serialPoll:	 Polled USART interface.
*
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTime.h"
#include "mod_led.h"
#include "twiRegSlave.h"
#include "serialPoll.h"

#define LED_DELAY		1000UL		// N * 1ms
#define TWI_DELAY		1000UL
#define SLAVE_ADRS		0x56
#define USART_DELAY		1000UL

volatile uint8_t rxRegister[16];
volatile uint8_t txRegister[16];

int main(void)
{
	uint32_t ledTime = 0UL;
	uint32_t twiTime = 0UL;
	uint32_t usartTime = 0UL;

	int loopCount = 0;
	
	init_time();		// set up clock and timers
	mod_led_init();		// set up LED pin
	twiRegSlaveInit(SLAVE_ADRS, rxRegister, 16, txRegister, 16);
	USART0_init(9600);
	USART3_init(9600);

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
	
	/* Replace with your application code */
	while (1)
	{
#if 1
		// Check every ms
		if( st_millis() > ledTime ) {
			ledTime += LED_DELAY;
			mod_led_toggle(250);
		}
#endif
#if 0
		// Demo TWI I2C
		if( st_millis() > twiTime ) {
			twiTime += TWI_DELAY;
			// TODO: Replace this with ADC read of temperature.
			twiSetRegister(1, loopCount>>8);
			twiSetRegister(2, loopCount++);
		}

		if( twiGetRegister(1) ) {
			mod_led_on();
		} else {
			mod_led_off();
		}
#endif
#if 0
		// Demo USART0
		if( st_millis() > usartTime ) {
			usartTime += USART_DELAY;
			// Send a 'tic' character.
			USART0_sendChar('*');
		}
		// Echo back any received character.
		if( USART0_isChar() ) {
			USART0_sendChar( USART0_recvChar() );
		}
#endif
#if 1
		// Demo USART3
		if( st_millis() > usartTime ) {
			usartTime += USART_DELAY;
			// Send a 'tic' character.
			USART3_sendChar('*');
		}
		// Echo back any received character.
		if( USART3_isChar() ) {
			USART3_sendChar( USART3_recvChar() );
		}
#endif
	}
}
