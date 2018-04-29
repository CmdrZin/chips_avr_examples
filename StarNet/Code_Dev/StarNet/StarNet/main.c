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
 * StarNet.c
 *
 * Created: 12/2/2016 2:07:50 PM
 * Author : Chip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "sysTimer.h"
#include "flash_table.h"

#define		LED_ON_TIME		10			// N * 100us

// Prototypes
void showALL();
void showBuffer();

void led_on( uint8_t ledNumber );

uint8_t buffer2[3] = {0x33, 0x33, 0x03 };
uint8_t buffer[3];


int main(void)
{
	uint8_t	shiftDelay = 10;

	st_init_tmr0();				// initialize timer

	sei();						// global enable interrupts

	while(1) {
#if 0
		showALL();
#else
		showBuffer();			// Takes 20ms if LED_ON_TIME = 100
		// Reload Buffer
		buffer[0] = buffer2[0];
		buffer[1] = buffer2[1];
		buffer[2] = buffer2[2];

		if(--shiftDelay == 0)
		{
			shiftDelay = 10;
			// Rotate buffer2[]
			buffer2[2] <<= 1;
			buffer2[2] |= (buffer2[1] & 0x80) ? 1 : 0;
			buffer2[1] <<= 1;
			buffer2[1] |= (buffer2[0] & 0x80) ? 1 : 0;
			buffer2[0] <<= 1;
			buffer2[0] |= (buffer2[2] & 0x10) ? 1 : 0;
		}
#endif
	}


// TEST CODE
	//DDRB |= (1<<PB1);			// set I/O as output.
//
    //while (1) 
    //{
		//if( GPIOR0 & (1<<DEV_100US_TIC) ) {
			//GPIOR0 &= ~(1<<DEV_100US_TIC);
			//++count;
			//if( count & 0x01 ) {
				//PORTB |= (1<<PB1);
			//} else {
				//PORTB &= ~(1<<PB1);
			//}
		//}
    //}
}

/* Displays */
// Turn on ALL LEDs for maximum brightness.
// 20ms if LED_ON_TIME = 10
void showALL() {
	uint8_t	waitTime;

	waitTime = LED_ON_TIME;			// N * 100us TIC
	
	for( int i=1; i<21; i++ ) {
		led_on( i );
		while( waitTime != 0 ) {
			if( GPIOR0 & (1<<DEV_100US_TIC) ) {
				GPIOR0 &= ~(1<<DEV_100US_TIC);		// reset flag
				--waitTime;
			}
		}
		waitTime = LED_ON_TIME;		// reset wait time.
	}
}

// Display Buffer
// 20ms if LED_ON_TIME = 10
void showBuffer() {
	uint8_t		data;
	uint16_t	waitTime;

	waitTime = LED_ON_TIME;			// N * 100us TIC
	
	for( int i=1; i<21; i++ ) {
		data = buffer[2];
		buffer[2] <<= 1;
		buffer[2] |= (buffer[1] & 0x80) ? 1 : 0;
		buffer[1] <<= 1;
		buffer[1] |= (buffer[0] & 0x80) ? 1 : 0;
		buffer[0] <<= 1;

		if( data & 0x08 ) {
			data = flash_get_led_index( i );
			led_on( data );
		} else {
			led_on( 0 );
		}
		while( waitTime != 0 ) {
			if( GPIOR0 & (1<<DEV_100US_TIC) ) {
				GPIOR0 &= ~(1<<DEV_100US_TIC);		// reset flag
				--waitTime;
			}
		}
		waitTime = LED_ON_TIME;		// reset wait time.
	}
}


/* Utilities */
void led_on( uint8_t ledNumber )
{
	static uint8_t lastDDR = 0;
	static uint8_t lastPORT = 0;

	DDRB = lastDDR;
	PORTB = lastPORT;

	// Calculate new patterns.
	lastDDR = flash_get_ddr_pattern( ledNumber );
	lastPORT = flash_get_port_pattern( ledNumber );
}


/* Pattern Tables */
const uint8_t ddrTable[] PROGMEM = {
	0b00000000,	//  0: OFF
	0b00000011,	//  D1
	0b00000011,	//  D2
	0b00000110,	//  D3
	0b00000110,	//  D4
	0b00001100,	//  D5
	0b00001100,	//  D6
	0b00011000,	//  D7
	0b00011000,	//  D8
	0b00010100,	//  D9
	0b00010100,	//  D10
	0b00001010,	//  D11
	0b00001010,	//  D12
	0b00010010,	//  D13
	0b00010010,	//  D14
	0b00000101,	//  D15
	0b00000101,	//  D16
	0b00001001,	//  D17
	0b00001001,	//  D18
	0b00010001,	//  D19
	0b00010001	//  D20
};

const uint8_t portTable[] PROGMEM = {
	0b00000000,	//  0: OFF
	0b00000010,	//  D1
	0b00000001,	//  D2
	0b00000100,	//  D3
	0b00000010,	//  D4
	0b00001000,	//  D5
	0b00000100,	//  D6
	0b00010000,	//  D7
	0b00001000,	//  D8
	0b00010000,	//  D9
	0b00000100,	//  D10
	0b00001000,	//  D11
	0b00000010,	//  D12
	0b00010000,	//  D13
	0b00000010,	//  D14
	0b00000100,	//  D15
	0b00000001,	//  D16
	0b00001000,	//  D17
	0b00000001,	//  D18
	0b00010000,	//  D19
	0b00000001	//  D20
};

// LED translate
const uint8_t ledTable[] PROGMEM = {
	0,	//  0: OFF
	1,	//  D1
	2,	//  D2
	16,	//  D3
	15,	//  D4
	6,	//  D5
	5,	//  D6
	20,	//  D7
	19,	//  D8
	7,	//  D9
	8,	//  D10
	18,	//  D11
	17,	//  D12
	12,	//  D13
	11,	//  D14
	10,	//  D15
	9,	//  D16
	13,	//  D17
	14,	//  D18
	3,	//  D19
	4	//  D20
};

