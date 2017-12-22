/*
 * dev_display.c
 *
 * Created: 6/3/2015		0.01	ndp
 *  Author: Chip
 * Revised: 7/26/2015		
 *
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>

#include "lcd_hdm16216h_5.h"

#include "dev_display.h"

#define LCD_REV	"0.1"

char buffer[2][8];				// display buffer..text is written here from I2C command and transfered to display during service.
bool newTextFlag;				// set after changing buffer data to cause display update.


void dev_display_init(uint8_t adrs)
{
	int tCount;
	char tBuf[18];
	
	lcd_init();

	for(int i=0; i<8; i++)
	{
		buffer[0][i] = ' ';
		buffer[1][i] = ' ';
	}
	
	// PoR banner
	tCount = snprintf(tBuf,17,"Rev:%3s I2C:0x%2X",LCD_REV,adrs);	// 17 for NULL term. Ware of OVERFLOWing tBuf!!!
	setText(tBuf, tCount);
}

// Copy the first 8 char into buffer 0 and the next 8 char into buffer 1.
void setText(char *text, uint8_t len)
{
	setTextOff(text, len, 0);
}

// Copy the first 8 char into buffer 0 starting at offset and the next 8 char into buffer 1.
void setTextOff(char *text, uint8_t len, uint8_t offset)
{
	int i;
	char *ptr = text;
	
	// Limit string length
	if(len > 16) len = 16;
	
	// Limit offset
	if(offset > 16) offset = 16;
	
	for(i=offset; i<8; i++)
	{
		// Copy text into buffer 0.
		buffer[0][i] = *ptr++;
	}
	
	for(; i<len; i++)
	{
		// Copy text into buffer 1.
		buffer[1][i-8] = *ptr++;
	}

	newTextFlag = true;
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

/* TODO: Redo to be more like printf()
 *
 * Display a line of text.
 * CMD: MOD 0x01 LINE c0 c1 ... c7 or a NULL
 * If less than 8 characters in command then fill the rest of the line with SPACE character.
 * LINE = 1 or 2
 */
void dev_display_set_text(void)
{
	uint8_t	 charCount = 0;
	uint8_t  index;
	uint8_t  bufSel = 0;
	char	data;

	// Copy text into buffer
	for(index=4;index<12;index++)
	{
		data = 'A'+index;
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
		// 4-bit writes
		lcd_ram_write(buff[index]);
		lcd_ram_write(buff[index]<<4);
		lcd_delay_40us();
	}
}
