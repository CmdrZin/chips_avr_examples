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
 * ButtonLED
 *
 * Created: 12/16/2020 3:33:10 PM
 * Author : Chip
 *	
 * The project uses the ATmega4809 Curiosity Nano Evaluation kit to demonstrate a
 * simple project framework. It uses the following support files:
 * sysTime     - timer function to provide millis().
 * mod_led     - general LED control.
 * I include these in just about every project I make.
 * 
 * The demo monitors the on-board button and flashes the on-board LED twice when the
 * button is pushed.
 * If the button is held down, the LED flashes three times two seconds after the
 * initial double flash.
 *
 * Two ways to implement the logic are used: If tree and State Machine.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "sysTime.h"
#include "mod_led.h"

void Blink2();
void Blink3();

void init_button();
bool isButtonPressed();

void stateMachine( bool buttonState );

#define DELAY_10MS	10UL
#define DELAY_100MS	100UL
#define CHECK_2SEC	200

typedef enum {IDLE, BUTTON_DOWN, BUTTON_UP} CTRL_STATE;

uint32_t lastTimeB2 = 0UL;
uint32_t lastTimeB3 = 0UL;

bool enableB2 = false;
bool enableB3 = false;

bool flag = false;
uint8_t count = 0;

CTRL_STATE state = IDLE;

int main(void)
{
	uint32_t lastTime = 0UL;

	init_time();		// set up clock and timers
	mod_led_init();		// set up LED pin
	init_button();

	sei();				// enable global interrupts

	// Use either the IF tree (1) or the State Machine (0)
#if 1
	// IF tree logic.
	while(true) {
		if(enableB2) Blink2();
		if(enableB3) Blink3();

		if(lastTime < st_millis()) {
			lastTime = st_millis() + DELAY_10MS;
			if(isButtonPressed()) {
				if(flag) {
					++count;
					if(count == CHECK_2SEC) {
						// Button held pressed for 2 seconds.
						enableB3 = true;
					}
					if(count > 210) {
						// Stop count from overflowing.
						count = 205;
					}
				} else {
					flag = true;
					count = 0;
					enableB2 = true;
				}
			} else {
				// Button not pressed
				++count;
				// De-bounce. Has to be released for 30 ms.
				if(count > 3) {
					flag = false;
				}
			}
		}

	}  // end while()
#else
	// State Machine logic.
	while(true) {
		if(enableB2) Blink2();
		if(enableB3) Blink3();

		if(lastTime < st_millis()) {
			lastTime = st_millis() + DELAY_10MS;
			stateMachine(isButtonPressed());
		}
	}	
#endif
}

// This State Machine implements the IF tree logic.
void stateMachine( bool buttonState )
{
	switch(state) {
		case IDLE:
			if(buttonState) {
				flag = true;
				state = BUTTON_DOWN;
				enableB2 = true;
			}
			break;
		
		case BUTTON_DOWN:
			if(buttonState) {
				// Still DOWN
				if(flag) {
					++count;
					if(count == CHECK_2SEC) {
						// Button held pressed for 2 seconds.
						enableB3 = true;
					}
					if(count > 210) {
						// Stop count from overflowing.
						count = 205;
					}
				}
			} else {
				// Button released.
				flag = false;
				count = 0;
				state = BUTTON_UP;
			}
			break;
		
		// Button must be released for 30ms or more.
		case BUTTON_UP:
			if(buttonState) {
				count = 0;
			} else {
				++count;
				if(count > 2) {
					state = IDLE;
				}
			}
			break;
			
		default:
			state = IDLE;
			break;
	}	
}


// Service blinking LED two times.
void Blink2()
{
	static bool active = false;
	static uint8_t count = 0;
	
	if(lastTimeB2 < st_millis()) {
		lastTimeB2 = st_millis() + DELAY_100MS;
		if(!active) {
			// Has just been enabled.
			active = true;
			count = 0;
			mod_led_on();
		} else {
			++count;
			if(count % 2 == 1) {
				mod_led_off();
			} else {
				mod_led_on();
			}
			if(count > 2) {
				active = false;
				enableB2 = false;
			}
		}
	}
}

// Service blinking LED three times.
void Blink3()
{
	static bool active = false;
	static uint8_t count = 0;
	
	if(lastTimeB3 < st_millis()) {
		lastTimeB3 = st_millis() + DELAY_100MS;
		if(!active) {
			// Has just been enabled.
			active = true;
			count = 0;
			mod_led_on();
		} else {
			++count;
			if(count % 2 == 1) {
				mod_led_off();
			} else {
				mod_led_on();
			}
			if(count > 4) {
				active = false;
				enableB3 = false;
			}
		}
	}
}



/* (see Section 16.5) Modify to match SW0 Button connection. [PORTF6]
 * Initialize the on-board button SW0 IO port and pin [PORTF6].
 * Pull-Up needed.
 */
void init_button()
{
	PORTF_DIRCLR = PIN6_bm;					// use DIRCLR to clear for input (default)
	PORTF_PIN6CTRL = PORT_PULLUPEN_bm ;		// enable pull-up bit.	
}

/* return TRUE if pressed. */
bool isButtonPressed()
{
	return !(PORTF_IN & PIN6_bm);
}
