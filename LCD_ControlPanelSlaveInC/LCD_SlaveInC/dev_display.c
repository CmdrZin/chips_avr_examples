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

#define USE_ASM 0			// 0:NO..1:YES

#if USE_ASM == 1
#include "lcd_hdm16216h_5_lib.h"
#else
#include "lcd_hdm16216h_5.h"
#endif

#include "dev_display.h"
#include "access.h"

char buffer[2][16];				// display buffer..text is written here from I2C command and transfered to display during service.
bool newTextFlag;				// set after changing buffer data to cause display update.


void dev_display_init(void)
{
#if USE_ASM
	lcd_lib_init();				// Use ASM lib
#else
	lcd_init();					// Use C lib
#endif

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
	buffer[0][5] = '0';
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
 * Display a line of text.
 * CMD: MOD 0x01 LINE c0 c1 ... c7 or a NULL
 * If less than 8 characters in command then fill the rest of the line with SPACE character.
 * LINE = 1 or 2
 */
void dev_display_set_text(void)
{
	uint8_t	 charCount = 0;
	uint8_t  index;
	uint8_t  bufSel;
	char	data;

	if(getMsgData(3) == 1) {
		bufSel = 0;
	} else {
		bufSel = 1;
	}
	// Copy text into buffer
	for(index=4;index<12;index++)
	{
		data = getMsgData(index);
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
