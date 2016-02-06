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
 */

/*
 * led_display.c
 *
 * Created: 1/13/2016	0.01	ndp
 *  Author: Chip
 * revision: 2/01/2016	0.02	ndp	Added support for Marquee scroll.
 *
 * This module manages the LED 8x8 display.
 * It will pull data from one of the two buffers an display one column at a time
 * at 5ms intervals. 25Hz refresh rate.
 */ 

#include <avr/io.h>

#include "icon_tables.h"
#include "flash_table.h"

#define COL_DDR		DDRD
#define COL_PORT	PORTD

#define ROW_DDR		DDRB
#define ROW_PORT	PORTB

#define LD_DELAY_COUNT	4			// n * 1ms

uint8_t		displayBuffer[2][8];	// dual display buffer for animations
uint8_t		buffer;					// buffer selection. 0 or 1
uint8_t		row;
uint8_t		ld_delay;
uint8_t		ld_row_pat;				// current row select pattern for column data displayed.
uint8_t		frames_displayed;		// number of frames the last icon has been displayed.

/*
 *
 */
void ld_init()
{
	// Setup IO
	COL_DDR = 0xFF;
	COL_PORT = 0xFF;				// active LOW to turn on column
	
	ROW_DDR = 0xFF;
	ROW_PORT = 0x00;				// active HIGH to turn on row
	// initialize variables
	buffer = 0;
	row = 0;
	frames_displayed = 0;
}

/*
 * Call each 1ms from mod_em_service.
 */
void ld_service()
{
	static uint8_t current_buffer;

	uint8_t cols;			// data for each given row
		
	// Check 5ms delay
	if( ld_delay != 0)
	{
		--ld_delay;
	}
	else
	{
		ld_delay = LD_DELAY_COUNT;		// reload
		if(row == 8)
		{
			// Finished display. Load a new one.
			current_buffer = buffer;
			row = 0;
			++frames_displayed;
		}

		ld_row_pat = 1 << row;								// active HIGH

		cols = ~(displayBuffer[current_buffer][row]);		// active LOW

		++row;

		//	Copy Column for Port
		COL_PORT = cols;
//		COL_PORT = 0b10111111;		// PORTD	ZYXWVUTS
		//	Set Row pins
		ROW_PORT = ld_row_pat;
//		ROW_PORT = 0b00000001;		// PORTB	HGFEDCBA
	}
}

/*
 * Copy icon[ val ] data into buffer[b].
 */
void ld_loadIcon( uint16_t val, uint8_t b )
{
	uint8_t index;
	
	index = b;
	
	// Don't overwrite current display buffer
	if( index == buffer )
	{
		( index == 0 ) ? (index = 1) : (index = 0);
	}

	// update. Takes affect on next full display scan.
	buffer = index;

	flash_copy8( val, icons, displayBuffer[index] );

	frames_displayed = 0;
}

/*
 * Copy buff[] data into buffer[b].
 */
void ld_directLoadIcon( uint8_t *buff )
{
	// Don't overwrite current display buffer
	if( buffer == 0 )
	{
		buffer = 1;
	}
	else
	{
		buffer = 0;
	}
	// Copy data
	for(int i=0; i<8; ++i)
	{
		displayBuffer[buffer][i] = buff[i];
	}

	frames_displayed = 0;
}

/*
 * Get frames displayed.
 */
uint8_t ld_getFramesDisplayed()
{
	return( frames_displayed );
}
