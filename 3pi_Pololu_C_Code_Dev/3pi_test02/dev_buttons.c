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
 * dev_buttons.c
 *
 * Created: 06/28/2016		0.01	ndp
 *  Author: Chip
 */ 

#include <avr/io.h>
#include "sysTimer.h"

#include "dev_buttons.h"

#define DEV_BUTTON_TIME		10

#define DEV_BUTTON_A_PORT	PORTB
#define DEV_BUTTON_A_DDR	DDRB
#define DEV_BUTTON_A_PIN	PINB
#define DEV_BUTTON_A_P		PORTB1

#define DEV_BUTTON_B_PORT	PORTB
#define DEV_BUTTON_B_DDR	DDRB
#define DEV_BUTTON_B_PIN	PINB
#define DEV_BUTTON_B_P		PORTB4

#define DEV_BUTTON_C_PORT	PORTB
#define DEV_BUTTON_C_DDR	DDRB
#define DEV_BUTTON_C_PIN	PINB
#define DEV_BUTTON_C_P		PORTB5

#define DEV_BUTTON_A_FLAG	0b00000001		// state returned
#define DEV_BUTTON_B_FLAG	0b00000010		// state returned
#define DEV_BUTTON_C_FLAG	0b00000100		// state returned

#define DEV_BUTTON_A_HIST	0b00010000		// set until released
#define DEV_BUTTON_B_HIST	0b00100000		// set until released
#define DEV_BUTTON_C_HIST	0b01000000		// set until released

uint8_t dev_button_status;					// button status.
uint8_t dev_button_delay;

/*
 * dev_buttons_init()
 *
 * Initialize I/O for Buttons
 *
 */
void dev_buttons_init()
{
	DEV_BUTTON_A_DDR &= ~(1<<DEV_BUTTON_A_P);			// Set as INPUT
	DEV_BUTTON_A_PORT |= (1<<DEV_BUTTON_A_P);			// Enable internal Pull-Up
	
	DEV_BUTTON_B_DDR &= ~(1<<DEV_BUTTON_B_P);			// Set as INPUT
	DEV_BUTTON_B_PORT |= (1<<DEV_BUTTON_B_P);			// Enable internal Pull-Up

	DEV_BUTTON_C_DDR &= ~(1<<DEV_BUTTON_C_P);			// Set as INPUT
	DEV_BUTTON_C_PORT |= (1<<DEV_BUTTON_C_P);			// Enable internal Pull-Up

	dev_button_status = 0;
	dev_button_delay = DEV_BUTTON_TIME;
}


/* 
 * Was button pressed?
 * button = 0..2 for A..C
 * Return 0:False  1:True..first read only, reset flag
 */

bool dev_button_isDown( DEV_BUTTON button )
{
	switch( button )
	{
		case DEV_BUTTON_A:
			if( dev_button_status & DEV_BUTTON_A_FLAG )
			{
				dev_button_status &= ~DEV_BUTTON_A_FLAG;		// clear flag.
				return true;
			}
			break;

		case DEV_BUTTON_B:
			if( dev_button_status & DEV_BUTTON_B_FLAG )
			{
				dev_button_status &= ~DEV_BUTTON_B_FLAG;		// clear flag.
				return true;
			}
			break;

		case DEV_BUTTON_C:
			if( dev_button_status & DEV_BUTTON_C_FLAG )
			{
				dev_button_status &= ~DEV_BUTTON_C_FLAG;		// clear flag.
				return true;
			}
			break;
	}
	
	return false;
}


/*
 * Was <num> button still pressed?
 * button = 0..2 for A..C
 * Return 0:False  1:True..first read only, reset flag
 */
bool dev_button_raw( DEV_BUTTON button )
{
	switch( button )
	{
		case DEV_BUTTON_A:
			if( dev_button_status & DEV_BUTTON_A_HIST )
			{
				return true;
			}
			break;

		case DEV_BUTTON_B:
			if( dev_button_status & DEV_BUTTON_B_HIST )
			{
				return true;
			}
			break;

		case DEV_BUTTON_C:
			if( dev_button_status & DEV_BUTTON_C_HIST )
			{
				return true;
			}
			break;
	}
	
	return false;
}


/*
 * Service buttons
 * Scan buttons every 100 ms.
 * If button
 */
void dev_buttons_service()
{
	if( GPIOR0 & (1<<DEV_10MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_10MS_TIC);				// clear flag
		if( --dev_button_delay == 0 )
		{
			dev_button_delay = DEV_BUTTON_TIME;

			if( !(DEV_BUTTON_A_PIN & (1<<DEV_BUTTON_A_P)) )		// Pressed?
			{
				// Is it still pressed?
				if( dev_button_status & DEV_BUTTON_A_HIST )
				{
					// yes..do nothing
				}
				else
				{
					// set both FLAG and HIST
					dev_button_status |= DEV_BUTTON_A_FLAG | DEV_BUTTON_A_HIST;
				}
			}
			else
			{
				// Released..clear HIST. FLAG is cleared when read.
					dev_button_status &= ~DEV_BUTTON_A_HIST;
			}

			if( !(DEV_BUTTON_B_PIN & (1<<DEV_BUTTON_B_P)) )		// Pressed?
			{
				// Is it still pressed?
				if( dev_button_status & DEV_BUTTON_B_HIST )
				{
					// yes..do nothing
				}
				else
				{
					// set both FLAG and HIST
					dev_button_status |= DEV_BUTTON_B_FLAG | DEV_BUTTON_B_HIST;
				}
			}
			else
			{
				// Released..clear HIST. FLAG is cleared when read.
				dev_button_status &= ~DEV_BUTTON_B_HIST;
			}

			if( !(DEV_BUTTON_C_PIN & (1<<DEV_BUTTON_C_P)) )		// Pressed?
			{
				// Is it still pressed?
				if( dev_button_status & DEV_BUTTON_C_HIST )
				{
					// yes..do nothing
				}
				else
				{
					// set both FLAG and HIST
					dev_button_status |= DEV_BUTTON_C_FLAG | DEV_BUTTON_C_HIST;
				}
			}
			else
			{
				// Released..clear HIST. FLAG is cleared when read.
				dev_button_status &= ~DEV_BUTTON_C_HIST;
			}
		}
	}
}
