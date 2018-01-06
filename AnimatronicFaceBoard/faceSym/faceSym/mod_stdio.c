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
 * mod_stdio.c
 *
 * Provides Output and Input services.
 * LCD print output
 * LED output
 * Button input
 *
 * Created: 7/23/2016 10:31:52 PM
 *  Author: Chip
 * Revised:	12/25/2017	0.01	ndp	Support for updated LCD display
 */ 
#include <avr/io.h>
#include <stdbool.h>

#include "mod_stdio.h"
#include "twi_I2CMaster.h"

#define MOD_LCD_CONTROL_I2C		0x5E

uint8_t msp_buff[20];

/*
 *
 */
void mod_stdio_print( char* buffer, uint8_t nbytes )
{
	// Pre-fill buffer with spaces
	for(uint8_t i=0; i<sizeof(msp_buff); i++) { msp_buff[i] = ' '; }

	msp_buff[0] = LCD_TEXT;
	msp_buff[1] = nbytes;
	// copy text
	for( uint8_t i=0; i<nbytes; i++ )
	{
		msp_buff[i+2] = buffer[i];
	}
	// Send packet.	
	tim_write( MOD_LCD_CONTROL_I2C, msp_buff, nbytes+2 );
}

/*
 *
 */
void mod_stdio_printOffset( char* buffer, uint8_t nbytes, uint8_t offset )
{
	msp_buff[0] = LCD_PTEXT;
	msp_buff[1] = nbytes;
	msp_buff[2] = offset;
	// copy text
	for( uint8_t i=0; i<nbytes; i++ )
	{
		msp_buff[i+3] = buffer[i];
	}
	// Send packet.
	tim_write( MOD_LCD_CONTROL_I2C, msp_buff, nbytes+3 );
}

// NO parameter checks
/*
 * GREEN: 0x20	RED: 0x21	YELLOW: 0x22
 */
void mod_stdio_led( uint8_t led, bool state )
{
	msp_buff[0] = led;
	msp_buff[1] = (state) ? 1 : 0;

	// Send packet.
	tim_write( led, msp_buff, 2 );
}
