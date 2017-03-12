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
 * dev_display.c
 *
 * Created: 6/3/2015		0.01	ndp
 *  Author: Chip
 * Revised: 7/26/2015		0.02	ndp
 * Revised: 1/31/2017		0.03	ndp		mod for direct LCD display.		
 *
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "lcd_hdm16216h_5.h"
#include "dev_display.h"

char buffer[2][16];				// display buffer..text is written here from I2C command and transfered to display during service.
bool newTextFlag;				// set after changing buffer data to cause display update.


void dev_display_init(void)
{
	lcd_init();

	newTextFlag = true;

	for(int i=0; i<8; i++)
	{
		buffer[0][i] = ' ';
		buffer[1][i] = ' ';
	}
	
	// PoR banner
	buffer[0][0] = 'R';	
	buffer[0][1] = 'e';
	buffer[0][2] = 'v';
	buffer[0][3] = ':';
	buffer[0][4] = '0';
	buffer[0][5] = '3';

	buffer[1][0] = '0';
	buffer[1][1] = '2';
	buffer[1][2] = '-';
	buffer[1][3] = '0';
	buffer[1][4] = '5';
	buffer[1][5] = '-';
	buffer[1][6] = '1';
	buffer[1][7] = '7';
}


/* Could be a timed update. */
void dev_display_service(void)
{
	if(newTextFlag)
	{
		// Send text buffer to display.
		dev_display_line_text(1, buffer[0]);
		dev_display_line_text(2, buffer[1]);
		newTextFlag = false;
	}
}

/*
 * Display a line of text. NULL terminated.
 * If less than 8 characters in command then fill the rest of the line with SPACE character.
 * line: 1 or 2
 */
void dev_display_set_text(uint8_t line, char* buf)
{
	uint8_t	 charCount = 0;
	uint8_t  index;
	uint8_t  bufSel;
	char	data;

	if(line == 1) {
		bufSel = 0;
	} else {
		bufSel = 1;
	}
	// Copy text into buffer
	for(index=0;index<8;index++)
	{
		data = buf[index];
		if(data != 0) {
			buffer[bufSel][charCount] = data;
			++charCount;
		} else {
			break;
		}
	}
	// Check for full line of characters. Fill spaces.
	if(charCount < 8) {
		for(;charCount<8;charCount++) {
			buffer[bufSel][charCount] = ' ';
		}
	}
	
	newTextFlag = true;
}


/* *** LCD Utilities *** */
void dev_display_line_text(uint8_t line, char buff[])
{
	uint8_t index;
	uint8_t ddadrs;
	
	(line == 1) ? (ddadrs=0) : (ddadrs=0x28);
	lcd_set_ddram(ddadrs);

	for(index=0; index<8; index++)
	{
		lcd_busy_check();
		// 4-bit writes
		lcd_ram_write(buff[index]);
		lcd_ram_write(buff[index]<<4);
//		lcd_delay_40us();
	}
}

void dev_bin2hex( char* buff, uint8_t val )
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

/*
 * Binary (8bit) to ASCII BCD ('0''0'-'9''9')
 * 
 */
 uint16_t dev_bin2BCD(uint8_t data)
 {
	uint16_t result = 0;
	uint8_t temp = data;

	for(uint8_t i=0; i<8; i++) {
		// test bits
		result += ((result & 0x0F) > 4) ? 3 : 0;
		result += ((result & 0xF0) > 0x40) ? 0x30 : 0;
		// shift into result
		result <<= 1;
		result |= (temp & 0x80) ? 1 : 0;
		// shift data
		temp <<= 1;
	}

	temp = (result & 0x0F) + 0x30;				// add ASCII '0'
	result = (result << 4) + 0x3000;
	result &= 0xFF00;
	result += temp;

	return result;
 }
