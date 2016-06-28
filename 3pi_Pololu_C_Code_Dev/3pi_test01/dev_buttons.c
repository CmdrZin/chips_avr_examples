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
 * io7SegLedsButtons.c
 *
 * Created: 11/25/2015		0.01	ndp
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "dev_buttons.h"


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
#define DEV_BUTTON_C_P		PORTB1


uint8_t db_status;	// button status.

/*
 * io_init()
 *
 * Initialize IO for LEDs and Buttons
 *
 */
void idl_init()
{
	IO_BUTTON_A_DDR &= ~(1<<IO_BUTTON_A_P);			// Set as INPUT
	IO_BUTTON_A_PORT |= (1<<IO_BUTTON_A_P);			// Enable internal Pull-Up
	
	IO_BUTTON_B_DDR &= ~(1<<IO_BUTTON_B_P);			// Set as INPUT
	IO_BUTTON_B_PORT |= (1<<IO_BUTTON_B_P);			// Enable internal Pull-Up

	IO_BUTTON_C_DDR &= ~(1<<IO_BUTTON_C_P);			// Set as INPUT
	IO_BUTTON_C_PORT |= (1<<IO_BUTTON_C_P);			// Enable internal Pull-Up

	db_status = 0;
}


/* 
 * Is <num> button pressed?
 * Return 0:False  1:True..first read  2:True..still down.
 */

uint8_t dev_button_isDown( DEV_BUTTON button );

#if 0
/*
 * Service display
 */
void idl_service()
{
	if( GPIOR0 & (1<<DEV_100MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_100MS_TIC);				// clear flag
		if( --idl_delay == 0 )
		{
			idl_delay = IDL_DIGIT_TIME;

			switch( idl_mode )
			{
				case IDL_OFF:
					idl_displayDigit( 0, IDL_OFF );
					break;
				
				case IDL_DL1:
					idl_displayDigit( idl_val1, IDL_DL1 );
					idl_mode = IDL_DL2;
					break;
					
				case IDL_DL2:
					idl_displayDigit( idl_val2, IDL_DL2 );
					idl_mode = IDL_DL1;
					break;
					
				default:
					idl_mode = IDL_OFF;
					break;
			}
		}
	}
}
#endif


/*
 * Scan UP and DOWN buttons.
 * b1:UP, b0:DOWN	0:open, 1:closed
 */
uint8_t idl_scanButtons()
{
	uint8_t result = 0;
	
	if( !(IO_BUTTON_DN_PIN & (1<<IO_BUTTON_DN_P)) )		// test for GND (pressed)
		result |= 0x01;
		
	if( !(IO_BUTTON_UP_PIN & (1<<IO_BUTTON_UP_P)) )
	result |= 0x02;

	return( result );
}
