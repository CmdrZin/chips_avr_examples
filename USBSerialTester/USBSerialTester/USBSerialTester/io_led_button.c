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
 * io_led_button.c
 *
 * LEDs are active LOW.
 *
 * Created: 7/24/2016		0.01	ndp
 *  Author: Chip
 * revised:	01/31/2017		0.02	ndp		For code development use.
 * revised: 02/16/2017		0.03	ndp		for Programmable Timer board rev1.1.
 * revised: 05/08/2017		0.04	ndp		for Programmable Timer board rev1.1..no button memory
 */ 


#include <avr/io.h>
#include <stdbool.h>

#include "io_led_button.h"

#include "sysTimer.h"


#define BUTTON_GREEN_DDR	DDRB			// A
#define BUTTON_GREEN_PORT	PORTB
#define BUTTON_GREEN_PIN	PINB
#define BUTTON_GREEN_P		PINB5

#define BUTTON_RED_DDR		DDRB			// B
#define BUTTON_RED_PORT		PORTB
#define BUTTON_RED_PIN		PINB
#define BUTTON_RED_P		PINB4

#define LED_GREEN_DDR		DDRD
#define LED_GREEN_PORT		PORTD
#define LED_GREEN_P			PORTD6

#define LED_RED_DDR			DDRD
#define LED_RED_PORT		PORTD
#define LED_RED_P			PORTD5

#define ILB_DELAY	100
uint8_t ilb_scanDelay = ILB_DELAY;			// N * 1ms

uint8_t			ilb_count;
MOD_IO_STATE	ilb_state;
uint8_t			ilb_buttons;
uint8_t			ilb_blink;
uint8_t			ilb_led;

/*
 *
 */
void mod_io_init()
{
	LED_GREEN_DDR |= (1<<LED_GREEN_P);			// set as output
	LED_RED_DDR |= (1<<LED_RED_P);				// set as output
	// Buttons default to INPUTs..activate pull-ups
	BUTTON_GREEN_PORT |= (1<<BUTTON_GREEN_P);
	BUTTON_RED_PORT |= (1<<BUTTON_RED_P);
	// Turn LEDs OFF
	ilb_setGreenLed(false);
	ilb_setRedLed(false);

	ilb_count = 0;
//	ilb_state = IO_TEST;
	ilb_state = IO_SCAN;
	ilb_buttons = 0;
	ilb_blink = 0;
	ilb_led = 0;
}

/*
 * Set Blink	N * 1ms
 */
void mod_io_setBlink( uint8_t val, uint8_t led )
{
	ilb_blink = val;
	ilb_led = led;
}

/*
 *
 */
void mod_io_service()
{
	static long lastTime = 0L;
	if(lastTime != st_millis())
	{
		lastTime = st_millis();

		// Blink N * 1ms
		if(ilb_blink != 0) {
			switch(ilb_led) {
				case IO_GREEN_LED:
					ilb_setGreenLed(true);
					break;

				case IO_RED_LED:
					ilb_setRedLed(true);
					break;

				case IO_BOTH_LED:
					ilb_setGreenLed(true);
					ilb_setRedLed(true);
					break;

				default:
					break;
			}
			--ilb_blink;
		} else {
			ilb_setGreenLed(false);
			ilb_setRedLed(false);
		}

		if( --ilb_scanDelay == 0)
		{
			ilb_scanDelay = ILB_DELAY;
			
			switch( ilb_state )
			{
				case IO_TEST:
					ilb_setGreenLed( (ilb_count & 0x01) == 0x01 );
					ilb_setRedLed( (ilb_count & 0x02) == 0x02 );
					if(++ilb_count > 24)
					{
						ilb_count = 0;
						ilb_state = IO_SCAN;
					}
					break;
					
				// Button scan and Blink once
				case IO_SCAN:
					ilb_scanButtons();
					break;
					
			}
		}
	}
}

/* set flag TRUE if button is released.
 * set state if button is pressed and AND with flag
 * flag state
 * xGRY xGRY
 */

// Return buttons state 00000GRY
uint8_t mod_io_getButtons()
{
	uint8_t temp;
	
	temp = ilb_buttons & 0x07;			// just return button states
	ilb_buttons &= 0x70;				// clear on read..leave flags alone

	return temp;
}

/*
 * flag TRUE if button has been pressed.
 * set FALSE if get called and button has not been released.
 */
void ilb_scanButtons()
{
	// Button has to be pressed during scan. No memory.
//	ilb_buttons = 0;				// clear

	if( BUTTON_GREEN_PIN & (1<<BUTTON_GREEN_P) )
	{
		// Button is UP..clear flag.
		ilb_buttons &= ~(IO_GREEN_BUTTON<<4);
	}
	else
	{
		// Button is DOWN..check flag.
		if( !(ilb_buttons & (IO_GREEN_BUTTON<<4)) )
		{
			ilb_buttons |= IO_GREEN_BUTTON;
		}
		// Set flag if button is down.
		ilb_buttons |= (IO_GREEN_BUTTON<<4);
	}

	if( BUTTON_RED_PIN & (1<<BUTTON_RED_P) )
	{
		// Button is UP..clear flag.
		ilb_buttons &= ~(IO_RED_BUTTON<<4);
	}
	else
	{
		// Button is DOWN..check flag.
		if( !(ilb_buttons & (IO_RED_BUTTON<<4)) )
		{
			ilb_buttons |= IO_RED_BUTTON;
		}
		// Set flag if button is down.
		ilb_buttons |= (IO_RED_BUTTON<<4);
	}
}

void ilb_setGreenLed( bool state )
{
	if( state )
	{
		LED_GREEN_PORT &= ~(1<<LED_GREEN_P);	// set LOW to turn ON
	}
	else
	{
		LED_GREEN_PORT |= (1<<LED_GREEN_P);		// set HIGH to turn OFF
	}
}

void ilb_setRedLed( bool state )
{
	if( state )
	{
		LED_RED_PORT &= ~(1<<LED_RED_P);		// set LOW to turn ON
	}
	else
	{
		LED_RED_PORT |= (1<<LED_RED_P);			// set HIGH to turn OFF
	}
}
