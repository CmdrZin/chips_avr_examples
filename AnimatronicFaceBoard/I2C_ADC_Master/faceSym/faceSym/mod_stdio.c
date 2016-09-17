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
 */ 
#include <avr/io.h>
#include <stdbool.h>

#include "mod_stdio.h"
#include "twi_I2CMaster.h"

#define MOD_LCD_CONTROL_I2C		0x60

#define MOD_LCD_DISPlAY		4
#define MOD_IO_LEDS_BUTTONS	5

uint8_t msp_makeHeader( uint8_t len );
void bin2hex(uint8_t* buff, uint8_t val);

uint8_t msp_buff[16];
uint8_t hexBuff[8];


/*
 * Line 1: Start at 0
 * Line 2: Start at 8
 */
void mod_stdio_print( uint8_t line, char* buffer, uint8_t nbytes )
{
	for(uint8_t i=0; i<16; i++) { msp_buff[i] = ' '; }

	msp_buff[0] = msp_makeHeader( nbytes+1 );				// data part of packet
	msp_buff[1] = MOD_LCD_DISPlAY;							// LCD display ID
	msp_buff[2] = 0x01;										// print CMD
	msp_buff[3] = line;
	// copy text
	for( uint8_t i=0; i<nbytes; i++ )
	{
		msp_buff[i+4] = buffer[i];
	}
	// Send packet.
	tim_write( MOD_LCD_CONTROL_I2C, msp_buff, nbytes+4 );
}

/*
 * Print 2 char Hex value.
 * TODO: Change this to a utility to return 2 HEX char.
 */
void mod_stdio_print2Hex( char* buff, uint8_t val1, uint8_t val2 )
{
	hexBuff[0] = ' ';
	hexBuff[1] = buff[0];
	hexBuff[2] = buff[1];
	hexBuff[3] = ':';
	bin2hex(&hexBuff[4], val1);
	bin2hex(&hexBuff[6], val2);
	mod_stdio_print(2, (char*)hexBuff, 8);
}

void mod_stdio_bin2hex( char* buff, uint8_t val )
{
	uint8_t temp;
	
	temp = val>>4;
	if(temp > 9) {
		buff[0] = temp - 10 + 'A';
		} else {
		buff[0] = temp + '0';
	}
	temp = val & 0x0F;
	if(temp > 9) {
		buff[1] = temp - 10 + 'A';
		} else {
		buff[1] = temp + '0';
	}
}

void mod_stdio_led( uint8_t led, bool state )
{
	msp_buff[0] = msp_makeHeader( 1 );
	msp_buff[1] = MOD_IO_LEDS_BUTTONS;
	msp_buff[2] = led;						// LED CMD code per define in header.
	msp_buff[3] = (state) ? 1 : 0;
	// Send packet.
	tim_write( MOD_LCD_CONTROL_I2C, msp_buff, 4 );
}

uint8_t msp_makeHeader( uint8_t len )
{
	uint8_t temp;
	
	temp = ((~len) << 4) & 0xF0;    // shift and mask
	temp = temp | (len & 0x0F);
	return (temp);
}
