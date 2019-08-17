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
 * core_functions.c
 *
 * Created: 8/8/2015	0.01	ndp
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>


#include "twiSlave.h"

#include "core_functions.h"

const char coreVersion[] PROGMEM = "0.01";		// add spaces to pad to 4 characters.
const char buildDate[] PROGMEM = __DATE__;		// 11 bytes
const char buildTime[] PROGMEM = __TIME__;		// 8 bytes


/*
 * Debug initialization.
 */
void core_init()
{
	return;
}

void core_service()
{
	// NO dynamic services at this time.
	return;
}


/* *** CORE ACCESS FUNCTIONS *** */

/*
 * Return build DATE stamp.
 * CMD: F0 FE 10
 */
void core_get_build_date()
{
	char data;
	uint8_t index;
	
	// Get data and put it into output fifo with device ID
	index = 0;
	twiTransmitByte( CORE_FUNCTIONS_ID );
	while( (data = pgm_read_byte(&(buildDate[index]))) != 0 && (index < 15) )
	{
		twiTransmitByte( data );
		++index;
	}
}

/*
 * Return build TIME stamp.
 * CMD: F0 FE 11
 */
void core_get_build_time()
{
	char data;
	uint8_t index;
	
	// Get data and put it into output fifo with device ID
	index = 0;
	twiTransmitByte( CORE_FUNCTIONS_ID );
	while( (data = pgm_read_byte(&(buildTime[index]))) != 0 && (index < 15) )
	{
		twiTransmitByte( data );
		++index;
	}
}

/*
 * Return core Version.
 * CMD: F0 FE 02
 */
void core_get_version()
{
	char data;
	uint8_t index;
	
	// Get data and put it into output fifo with device ID
	index = 0;
	twiTransmitByte( CORE_FUNCTIONS_ID );
	while( (data = pgm_read_byte(&(coreVersion[index]))) != 0 && (index < 15) )
	{
		twiTransmitByte( data );
		++index;
	}
}
