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
 * Created: 7/24/2016 10:11:26 AM
 *  Author: Chip
 */ 


#include <avr/io.h>
#include <stdbool.h>

#include "io_led_button.h"

#include "sysTimer.h"
#include "twiSlave.h"
#include "access.h"


#define BUTTON_GREEN_DDR	DDRB
#define BUTTON_GREEN_PORT	PORTB
#define BUTTON_GREEN_PIN	PINB
#define BUTTON_GREEN_P		PINB3

#define BUTTON_RED_DDR		DDRB
#define BUTTON_RED_PORT		PORTB
#define BUTTON_RED_PIN		PINB
#define BUTTON_RED_P		PINB4

#define BUTTON_YELLOW_DDR	DDRB
#define BUTTON_YELLOW_PORT	PORTB
#define BUTTON_YELLOW_PIN	PINB
#define BUTTON_YELLOW_P		PINB5

#define LED_GREEN_DDR	DDRB
#define LED_GREEN_PORT	PORTB
#define LED_GREEN_P		PORTB6

#define LED_RED_DDR		DDRB
#define LED_RED_PORT	PORTB
#define LED_RED_P		PORTB7

#define LED_YELLOW_DDR	DDRB
#define LED_YELLOW_PORT	PORTB
#define LED_YELLOW_P	PORTB0

#define ILB_DELAY	100
uint8_t ilb_scanDelay = ILB_DELAY;			// N * 1ms

uint8_t			ilb_count;
MOD_IO_STATE	ilb_state;
uint8_t			ilb_buttons;

/*
 *
 */
void mod_io_init()
{
	LED_GREEN_DDR |= (1<<LED_GREEN_P);			// set as output
	LED_RED_DDR |= (1<<LED_RED_P);				// set as output
	LED_YELLOW_DDR |= (1<<LED_YELLOW_P);		// set as output
	// Buttons default to INPUTs..activate pull-ups
	BUTTON_GREEN_PORT |= (1<<BUTTON_GREEN_P);
	BUTTON_RED_PORT |= (1<<BUTTON_RED_P);
	BUTTON_YELLOW_PORT |= (1<<BUTTON_YELLOW_P);
	
	ilb_count = 0;
	ilb_state = IO_TEST;
	ilb_buttons = 0;
}

/*
 *
 */
void mod_io_service()
{
	if( GPIOR0 & (1<<DEV_1MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_1MS_TIC);
		if( --ilb_scanDelay == 0)
		{
			ilb_scanDelay = ILB_DELAY;
			
			switch( ilb_state )
			{
				case IO_TEST:
					ilb_setGreenLed( (ilb_count & 0x01) == 0x01 );
					ilb_setRedLed( (ilb_count & 0x02) == 0x02 );
					ilb_setYellowLed( (ilb_count & 0x04) == 0x04 );
					if(++ilb_count > 24)
					{
						ilb_count = 0;
						ilb_state = IO_SCAN;
					}
					break;
					
				// Button scan
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
void mod_io_getButtons()
{
	uint8_t temp;
	
	temp = ilb_buttons & 0x07;			// just return button states
	ilb_buttons &= 0x70;				// clear on read..leave flags alone

	twiTransmitByte( MOD_IO_LED_BUTTON_ID );
	twiTransmitByte( temp );
}

void mod_io_setGreenLed()
{
	ilb_setGreenLed( getMsgData(3) == 0x01 );
}

void mod_io_setRedLed()
{
	ilb_setRedLed( getMsgData(3) == 0x01 );
}

void mod_io_setYellowLed()
{
	ilb_setYellowLed( getMsgData(3) == 0x01 );
}

/*
 * flag TRUE if button has been pressed.
 * set FALSE if get called and button has not been released.
 */
void ilb_scanButtons()
{
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

	if( BUTTON_YELLOW_PIN & (1<<BUTTON_YELLOW_P) )
	{
		// Button is UP..clear flag.
		ilb_buttons &= ~(IO_YELLOW_BUTTON<<4);
	}
	else
	{
		// Button is DOWN..check flag.
		if( !(ilb_buttons & (IO_YELLOW_BUTTON<<4)) )
		{
			ilb_buttons |= IO_YELLOW_BUTTON;
		}
		// Set flag if button is down.
		ilb_buttons |= (IO_YELLOW_BUTTON<<4);
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

void ilb_setYellowLed( bool state )
{
	if( state )
	{
		LED_YELLOW_PORT &= ~(1<<LED_YELLOW_P);		// set LOW to turn ON
	}
	else
	{
		LED_YELLOW_PORT |= (1<<LED_YELLOW_P);		// set HIGH to turn OFF
	}
}
