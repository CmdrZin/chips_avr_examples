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
 *
 * lcd_hdm16216h_5.c
 *
 * Created: 6/4/2015	ndp		0.01
 *  Author: Chip
 * revised:	7/1/2016	ndp		0.02		change pin assignments to match 3pi
 */ 

#include <avr/io.h>

#include "lcd_hdm16216h_5.h"

// Local defines
#define LCD_DISPLAY_RS_DDR	DDRD
#define LCD_DISPLAY_RS_PORT PORTD
#define LCD_DISPLAY_RS_PIN	PORTD2

#define LCD_DISPLAY_RW_DDR	DDRB
#define LCD_DISPLAY_RW_PORT PORTB
#define LCD_DISPLAY_RW_PIN	PORTB0

#define LCD_DISPLAY_E_DDR	DDRD
#define LCD_DISPLAY_E_PORT	PORTD
#define LCD_DISPLAY_E_PIN	PORTD4

#define LCD_DISPLAY_DATA1_DDR		DDRB
#define LCD_DISPLAY_DATA1_PORT		PORTB
#define LCD_DISPLAY_DATA1_PORTIN	PINB
#define LCD_DISPLAY_DATA1_DB4		PORTB1
#define LCD_DISPLAY_DATA1_DB5		PORTB4
#define LCD_DISPLAY_DATA1_DB6		PORTB5
#define LCD_DISPLAY_DATA1_MASK		0b00110010

#define LCD_DISPLAY_DATA2_DDR		DDRD
#define LCD_DISPLAY_DATA2_PORT		PORTD
#define LCD_DISPLAY_DATA2_PORTIN	PIND
#define LCD_DISPLAY_DATA2_DB7		PORTD7
#define LCD_DISPLAY_DATA2_MASK		0b10000000

#define LCD_CLEAR_DISPLAY			0x01	// Write 0x20 to DDRAM and reset AC to 00. [1.52ms]
#define LCD_RETURN_HOME				0x02	// Reset AC to 00 and return cursor to home. [1.52ms]
#define LCD_SET_ENTRY_MODE			0x04	// b1: I/D, b0: S Set cursor move and display shift. [37us]
#define LCD_DISPLAY_ONOFF			0x08	// b2: Display, b1: Cursor, b0: Cursor position.  [37us]
#define LCD_CURSOR_DISPLAY_SHIFT	0x10	// b3: Select, b2: R/L. [37us]
#define LCD_SET_FUNCTION			0x20	// b4: Interface, b3: Lines, b2: Font. [37us]
#define LCD_SF_DL	0
#define LCD_SF_N	3
#define LCD_SET_CGRAM_ADRS			0x40	// b5:0 CGRAM Address. [37us]
#define LCD_SET_DDRAM_ADRS			0x80	// b6:0 DDRAM Address. [37us]

#define LCD_SHIFT_CURSOR_LEFT	0b00000000
#define LCD_SHIFT_CURSOR_RIGHT	0b00000100
#define LCD_SHIFT_DISPLAY_LEFT	0b00001000
#define LCD_SHIFT_DISPLAY_RIGHT	0b00001100

#define LCD_DELAY_1US_COUNT		5
#define LCD_DELAY_40US_COUNT	200

/*
 * Initialize HDM16216H-5 for 4bit data.
 */
void lcd_init()
{
	lcd_delay_1ms(50);			// wait 50ms after reset.
	lcd_write_ins_once(LCD_SET_FUNCTION | (1<<LCD_SF_DL));
	lcd_delay_1ms(5);
	lcd_write_ins_once(LCD_SET_FUNCTION | (1<<LCD_SF_DL));
	lcd_delay_40us();
	lcd_delay_40us();
	lcd_delay_40us();
	lcd_write_ins_once(LCD_SET_FUNCTION | (1<<LCD_SF_DL));
	lcd_delay_40us();
	lcd_write_ins_once(LCD_SET_FUNCTION | (0<<LCD_SF_DL));		// set for 4bit
	lcd_delay_40us();
	lcd_write_ins_nochk(LCD_SET_FUNCTION | (0<<LCD_SF_DL) | (1<<LCD_SF_N));
	
}


/*
 * These delays are used instead of assuming a timer is available.
 * Look at ASM code listing to count instruction in loop.
 * Based on 20 MHz clock.
 */
void lcd_delay_1us()
{
	uint8_t count;
	
	for(count=0; count<LCD_DELAY_1US_COUNT; count++)
	{
		// Adjust for ~1us delay at 20MHz. Could be based on a CPU_CLOCK define.
		asm("nop");
	}
	
}

void lcd_delay_40us()
{
	uint8_t count;
	
	for(count=0; count<LCD_DELAY_40US_COUNT; count++)
	{
		// Adjust for ~40us delay at 20MHz. Could be based on a CPU_CLOCK define.
		asm("nop");
	}
	
}

void lcd_delay_1ms(uint8_t msDelay)
{
	uint8_t count;
	uint8_t loopcnt;
	
	for(loopcnt=0; loopcnt<msDelay; loopcnt++)
	{
		for(count=0; count<25; count++)
		{
			lcd_delay_40us();
		}
	}
}

void lcd_write_ins_chk(uint8_t data)
{
	lcd_busy_check();						// Blocking call if BUSY.
	lcd_write_ins_nochk(data);
}

/*
 * This take an 8bit command and outputs the upper 4bits then the lower 4bits.
 */
void lcd_write_ins_nochk(uint8_t data)
{
	LCD_DISPLAY_RS_PORT &= !(1<<LCD_DISPLAY_RS_PIN);		// RS=0
	LCD_DISPLAY_RW_PORT &= !(1<<LCD_DISPLAY_RW_PIN);		// RW=0
	// output upper bits
	lcd_write_ins_once(data>>4);
	// output lower bits
	lcd_write_ins_once(data & 0x0F);
}

void lcd_write_ins_once(uint8_t data)
{
	lcd_cmd_write(data);
}

void lcd_busy_check()
{
	uint8_t busy = 0x80;
	while( busy != 0 )
	{
		busy = lcd_cmd_read() & 0x80;
	}
}

/* *** 3pi LCD data lines are NOT sequencial *** */
/*
 *		DB7	->	PD7		d7 . d3
 *		DB6	->	PB5		d6 . d2
 *		DB5	->	PB4		d5 . d1
 *		DB4	->	PB1		d4 . d0
 *
 * 4 bit data pass to/from LCD
 * PORTB:  7 6 5 4 3 2 1 0
 * Data:       2 1     0
 * PORTD:  7 6 5 4 3 2 1 0
 * Data:   3
 *
 * PORTB_MASK:	00110010
 * PORTD_MASK:	10000000
 *
 *	dataH = data>>4
 *	dataL = data & 0x0F
 *
 *	Output
 *		Set as OUTPUT
 *			LCD_DISPLAY_DATA1_DDR.LCD_DISPLAY_DATA1_DB4
 *			LCD_DISPLAY_DATA1_DDR.LCD_DISPLAY_DATA1_DB5
 *			LCD_DISPLAY_DATA1_DDR.LCD_DISPLAY_DATA1_DB6
 *			LCD_DISPLAY_DATA2_DDR.LCD_DISPLAY_DATA2_DB7
 *		Set per bit
 *			LCD_DISPLAY_DATA1_PORT.LCD_DISPLAY_DATA1_DB4 = d0 or d4
 *			LCD_DISPLAY_DATA1_PORT.LCD_DISPLAY_DATA1_DB5 = d1 or d5
 *			LCD_DISPLAY_DATA1_PORT.LCD_DISPLAY_DATA1_DB6 = d2 or d6
 *			LCD_DISPLAY_DATA2_PORT.LCD_DISPLAY_DATA2_DB7 = d3 or d7
 *		Set E
 *		Wait
 *		Clear E
 *
 *	Input
 *		Set as INPUT
 *			LCD_DISPLAY_DATA1_DDR.LCD_DISPLAY_DATA1_DB4
 *			LCD_DISPLAY_DATA1_DDR.LCD_DISPLAY_DATA1_DB5
 *			LCD_DISPLAY_DATA1_DDR.LCD_DISPLAY_DATA1_DB6
 *			LCD_DISPLAY_DATA2_DDR.LCD_DISPLAY_DATA2_DB7
 *		Set E
 *		Set data per bit
 *			d0 or d4 = LCD_DISPLAY_DATA1_PORT.LCD_DISPLAY_DATA1_DB4
 *			d1 or d5 = LCD_DISPLAY_DATA1_PORT.LCD_DISPLAY_DATA1_DB5
 *			d2 or d6 = LCD_DISPLAY_DATA1_PORT.LCD_DISPLAY_DATA1_DB6
 *			d3 or d7 = LCD_DISPLAY_DATA2_PORT.LCD_DISPLAY_DATA2_DB7
 *		Clear E
 *
 * NOTE: PD7 is also LED line. Restore DDRD7 and PORTD7 after use.
 */


/*
 * Only used for BUSY Flag check.
 */
#if 0
uint8_t lcd_cmd_read()
{
	uint8_t temp;

	LCD_DISPLAY_RS_PORT &= ~(1<<LCD_DISPLAY_RS_PIN);		// RS=0
	LCD_DISPLAY_RW_PORT |= (1<<LCD_DISPLAY_RW_PIN);			// RW=1
	// Set up data
	LCD_DISPLAY_DATA_DDR &= 0xF0;							// set as input
	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 360ns width. 4 clocks @ 8MHz.
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	temp = LCD_DISPLAY_DATA_PORTIN;							// read
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// clear E=0.

	return( temp );
}
#else
/*
 * MSb first then LSb
 * PORTD 7, PORTB 541 for d3:0
 */
uint8_t lcd_cmd_read()
{
	uint8_t temp;
	uint8_t oldDDRD, oldDDRB;
	uint8_t oldPORTD, oldPORTB;
	uint8_t result;

	// Save current pin I/O configuration.
	oldDDRD = DDRD;
	oldDDRB = DDRB;
	// Save current pin output.
	oldPORTD = PORTD;
	oldPORTB = PORTB;

	// Set up IO. Control pins are outputs.
	LCD_DISPLAY_RS_DDR |= (1<<LCD_DISPLAY_RS_PIN);
	LCD_DISPLAY_RW_DDR |= (1<<LCD_DISPLAY_RW_PIN);
	LCD_DISPLAY_E_DDR |= (1<<LCD_DISPLAY_E_PIN);
	// Data IO is set to input or output as needed.

	LCD_DISPLAY_RS_PORT &= ~(1<<LCD_DISPLAY_RS_PIN);		// RS=0
	LCD_DISPLAY_RW_PORT |= (1<<LCD_DISPLAY_RW_PIN);			// RW=1

	// Set up data
	LCD_DISPLAY_DATA1_DDR &= ~LCD_DISPLAY_DATA1_MASK;		// set as inputs
	LCD_DISPLAY_DATA2_DDR &= ~LCD_DISPLAY_DATA2_MASK;
	
	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 360ns width.
	lcd_delay_1us();
	temp = LCD_DISPLAY_DATA1_PORTIN;	// read d6:4
	result  = temp<<1 & 0b01100000;
	result |= temp<<3 & 0b00010000;
	temp = LCD_DISPLAY_DATA2_PORTIN;	// read d7
	result |= temp<<4 & 0b10000000;
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// clear E=0.
	// 1.2us delay to next E rising edge.
	lcd_delay_1us();

	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 360ns width.
	lcd_delay_1us();
	temp = LCD_DISPLAY_DATA1_PORTIN;	// read d2:0
	result |= temp>>3 & 0b00000110;
	result |= temp>>1 & 0b00000001;
	temp = LCD_DISPLAY_DATA2_PORTIN;	// read d3
	result |= temp>>4 & 0b00001000;
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// clear E=0.

	// Restore pin I/O configuration.
	DDRD = oldDDRD;
	DDRB = oldDDRB;
	// Restore pin output.
	PORTD = oldPORTD;
	PORTB = oldPORTB;

	return( result );
}
#endif


/*
 * Diagram shows 360ns data setup max after rising edge of E.
 * Just setup data BEFORE raising E.
 * Hold time after E falls is 10ns. One nop will do.
 * ASSUMES lower four bits are data lines.
 */
#if 0
void lcd_cmd_write(uint8_t data)
{
	uint8_t temp;
	
	LCD_DISPLAY_RS_PORT &= ~(1<<LCD_DISPLAY_RS_PIN);		// RS=0
	LCD_DISPLAY_RW_PORT &= ~(1<<LCD_DISPLAY_RW_PIN);		// RW=0
	// Set up data
	LCD_DISPLAY_DATA_DDR |= 0x0F;							// set as output
	temp = LCD_DISPLAY_DATA_PORT & 0xF0;					// clear bits first
	LCD_DISPLAY_DATA_PORT = temp | (data & 0x0F);			// then OR in data.
	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	asm("nop");
	asm("nop");
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// clear E=0.
}
#else
/*
 * This is a 4bit write. ASSUMES lower four bits are data.
 * MSb first then LSb
 * PORTD 7, PORTB 541 for d3:0
 */
void lcd_cmd_write(uint8_t data)
{
	uint8_t temp;
	uint8_t oldDDRD, oldDDRB;
	uint8_t oldPORTD, oldPORTB;
	uint8_t result;

	// Save current pin I/O configuration.
	oldDDRD = DDRD;
	oldDDRB = DDRB;
	// Save current pin output.
	oldPORTD = PORTD;
	oldPORTB = PORTB;

	// Set up IO. Control pins are outputs.
	LCD_DISPLAY_RS_DDR |= (1<<LCD_DISPLAY_RS_PIN);
	LCD_DISPLAY_RW_DDR |= (1<<LCD_DISPLAY_RW_PIN);
	LCD_DISPLAY_E_DDR |= (1<<LCD_DISPLAY_E_PIN);
	// Data IO is set to input or output as needed.

	// Set up data d2:0 -> 54 1
	result  = data<<3 & 0b00110000;			// d2:1
	result |= data<<1 & 0b00000010;			// d0
	LCD_DISPLAY_DATA1_DDR |= LCD_DISPLAY_DATA1_MASK;		// set as output
	temp = oldPORTB & ~LCD_DISPLAY_DATA1_MASK;				// clear bits first
	LCD_DISPLAY_DATA1_PORT = temp | result;					// then OR in data.
	// Set up data d3 -> 7
	result  = data<<4 & 0b10000000;			// d3
	LCD_DISPLAY_DATA2_DDR |= LCD_DISPLAY_DATA2_MASK;		// set as output
	temp = oldPORTD & ~LCD_DISPLAY_DATA2_MASK;				// clear bits first
	LCD_DISPLAY_DATA2_PORT = temp | result;					// then OR in data.
	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	lcd_delay_1us();
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// clear E=0.

	// Restore pin I/O configuration.
	DDRD = oldDDRD;
	DDRB = oldDDRB;
	// Restore pin output.
	PORTD = oldPORTD;
	PORTB = oldPORTB;
}
#endif


uint8_t lcd_ram_read()
{
	return 0;
}

/*
 * Diagram shows 360ns data setup max after rising edge of E.
 * Just setup data BEFORE raising E.
 * Hold time after E falls is 10ns. One nop will do.
 */
#if 0
void lcd_ram_write(uint8_t data)
{
	uint8_t temp;
	
	LCD_DISPLAY_RS_PORT |= (1<<LCD_DISPLAY_RS_PIN);			// RS=1
	LCD_DISPLAY_RW_PORT &= ~(1<<LCD_DISPLAY_RW_PIN);		// RW=0
	// Set up data
	LCD_DISPLAY_DATA_DDR |= 0x0F;							// set as output
	temp = LCD_DISPLAY_DATA_PORT & 0xF0;					// clear bits first
	LCD_DISPLAY_DATA_PORT = temp | (data & 0x0F);			// then OR in data.
	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	asm("nop");
	asm("nop");
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// set E=0.
}
#else
/*
 * This is a 4bit write. ASSUMES lower four bits are data.
 * MSb first then LSb
 * PORTD 7, PORTB 541 for d3:0
 */
void lcd_ram_write(uint8_t data)
{
	uint8_t temp;
	uint8_t oldDDRD, oldDDRB;
	uint8_t oldPORTD, oldPORTB;
	uint8_t result;

	// Save current pin I/O configuration.
	oldDDRD = DDRD;
	oldDDRB = DDRB;
	// Save current pin output.
	oldPORTD = PORTD;
	oldPORTB = PORTB;

	// Set up IO. Control pins are outputs.
	LCD_DISPLAY_RS_DDR |= (1<<LCD_DISPLAY_RS_PIN);
	LCD_DISPLAY_RW_DDR |= (1<<LCD_DISPLAY_RW_PIN);
	LCD_DISPLAY_E_DDR |= (1<<LCD_DISPLAY_E_PIN);
	// Data IO is set to input or output as needed.
	
	LCD_DISPLAY_RS_PORT |= (1<<LCD_DISPLAY_RS_PIN);			// RS=1
	LCD_DISPLAY_RW_PORT &= ~(1<<LCD_DISPLAY_RW_PIN);		// RW=0

	// Set up data d2:0 -> 54 1
	result  = data<<3 & 0b00110000;			// d2:1
	result |= data<<1 & 0b00000010;			// d0
	LCD_DISPLAY_DATA1_DDR |= LCD_DISPLAY_DATA1_MASK;		// set as output
	temp = oldPORTB & ~LCD_DISPLAY_DATA1_MASK;				// clear bits first
	LCD_DISPLAY_DATA1_PORT = temp | result;					// then OR in data.
	// Set up data d3 -> 7
	result  = data<<4 & 0b10000000;			// d3
	LCD_DISPLAY_DATA2_DDR |= LCD_DISPLAY_DATA2_MASK;		// set as output
	temp = oldPORTD & ~LCD_DISPLAY_DATA2_MASK;				// clear bits first
	LCD_DISPLAY_DATA2_PORT = temp | result;					// then OR in data.
	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	lcd_delay_1us();
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// clear E=0.

	// Restore pin I/O configuration.
	DDRD = oldDDRD;
	DDRB = oldDDRB;
	// Restore pin output.
	PORTD = oldPORTD;
	PORTB = oldPORTB;
}
#endif
