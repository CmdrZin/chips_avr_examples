/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * io_4x27SegLeds.c
 *
 * Created: 06/29/2017		0.01	ndp
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "io_4x27SegLeds.h"


#define IO_RESET_PORT		PORTC
#define IO_RESET_DDR		DDRC
#define IO_RESET_P			PC6

#define IO_SEG_PORT			PORTD
#define IO_SEG_DDR			DDRD

#define IO_LINES_PORT		PORTB
#define IO_LINES_DDR		DDRB
#define IO_LINE_1_D1		PB0
#define IO_LINE_1_D2		PB1
#define IO_LINE_1_D3		PB2
#define IO_LINE_1_D4		PB3
#define IO_LINE_2_D1		PB4
#define IO_LINE_2_D2		PB5
#define IO_LINE_2_D3		PB6
#define IO_LINE_2_D4		PB7

#define IDL_DIGIT_TIME		3		// N * 1ms

const uint8_t isl_segTable[] PROGMEM;

/*
 * isl_init()
 *
 * Initialize IO for LEDs
 *
 */
void isl_init()
{
	IO_RESET_DDR &= ~(1<<IO_RESET_P);					// Set as INPUT
	IO_RESET_PORT |= (1<<IO_RESET_P);					// Enable internal Pull-Up

	IO_SEG_PORT = 0;									// Disable segments
	IO_SEG_DDR = 0xFF;									// set as output
	
	IO_LINES_PORT = 0;
	IO_LINES_DDR = 0xFF;
}

/* 
 * isl_getPattern()
 *
 * Return 7-segment pattern for value.
 * Value is 8-bit to support special characters.
 */
uint8_t isl_getPattern( uint8_t val )
{
	uint16_t sAdrs = (uint16_t)isl_segTable;
	
	sAdrs += val;

	return ( pgm_read_byte( sAdrs ) );
}

/* 
 * isl_update()
 *
 * Display segments for value.
 * Value is 8-bit to support special characters.
 */
void isl_update( uint8_t digit, uint8_t pattern )
{
	// Turn OFF all LEDs
	IO_LINES_PORT = 0;
	// Output new pattern
	IO_SEG_PORT = pattern;
	// Turn on LED digit
	IO_LINES_PORT = (1<<digit);
}

/* *** LED Segment TABLE *** */
/*
 *      A
 *    F   B
 *      G
 *    E   C
 *      D   dp
 *
 *    dGFEDCBA
 */
const uint8_t isl_segTable[] PROGMEM = {
	0b00111111,	//  0
	0b00000110,	//  1
	0b01011011,	//  2
	0b01001111,	//  3
	0b01100110,	//  4
	0b01101101,	//  5
	0b01111101,	//  6
	0b00000111,	//  7
	0b01111111,	//  8
	0b01100111,	//  9
	0b01110111,	//  A
	0b01111100,	//  B
	0b00111001,	//  C
	0b01011110,	//  D
	0b01111001,	//  E
	0b01110001,	//  F

	0b00000000,	//  blank
	0b01000000,	//  -
	0b01110100,	//  o
	0b00010110,	//  L
	0b01000100	//  r
};
