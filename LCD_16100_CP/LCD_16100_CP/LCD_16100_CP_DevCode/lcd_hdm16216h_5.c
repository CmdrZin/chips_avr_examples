/*
 * lcd_hdm16216h_5.c
 *
 * Specific to ST7066U LCD controller chip - Single Line
 *
 * Created: 6/4/2015 2:39:41 PM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "lcd_hdm16216h_5.h"

// Local defines
#define LCD_DISPLAY_RS_DDR	DDRC
#define LCD_DISPLAY_RS_PORT PORTC
#define LCD_DISPLAY_RS_PIN	PORTC3

#define LCD_DISPLAY_RW_DDR	DDRC
#define LCD_DISPLAY_RW_PORT PORTC
#define LCD_DISPLAY_RW_PIN	PORTC2

#define LCD_DISPLAY_E_DDR	DDRC
#define LCD_DISPLAY_E_PORT	PORTC
#define LCD_DISPLAY_E_PIN	PORTC1

#define LCD_DISPLAY_DATA_DDR	DDRD
#define LCD_DISPLAY_DATA_PORT	PORTD
#define LCD_DISPLAY_DATA_PORTIN	PIND

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

#define LCD_DELAY_40US_COUNT	80

/*
 * Initialize HDM16216H-5 for 4bit data.
 */
void lcd_init()
{
	// Set up IO. Control pins LOW.
	LCD_DISPLAY_RS_PORT &= ~(1<<LCD_DISPLAY_RS_PIN);
	LCD_DISPLAY_RW_PORT &= ~(1<<LCD_DISPLAY_RW_PIN);
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);

	// Set up IO. Control pins are outputs.
	LCD_DISPLAY_RS_DDR |= (1<<LCD_DISPLAY_RS_PIN);
	LCD_DISPLAY_RW_DDR |= (1<<LCD_DISPLAY_RW_PIN);
	LCD_DISPLAY_E_DDR |= (1<<LCD_DISPLAY_E_PIN);
	// Data IO is set to input or output as needed.

	lcd_delay_1ms(50);			// wait 50ms after reset.
	lcd_write_ins_once(0x30);		// set for 8bit
	lcd_delay_1ms(5);
	lcd_write_ins_once(0x20);		// set to 4 bit
	lcd_delay_40us();

	lcd_write_ins_nochk(0x28);		// 2 lines
	lcd_delay_40us();
	lcd_write_ins_nochk(0x0E);		// Display:ON, Cursor:ON, Blink:OFF
	lcd_delay_40us();
	lcd_write_ins_nochk(0x01);
	lcd_delay_1ms(2);
	lcd_write_ins_nochk(0x06);
	lcd_delay_40us();

}

void lcd_set_ddram( uint8_t adrs )
{
	uint8_t temp;
	
	temp = adrs & 0x3F;			// mask data
	temp |= LCD_SET_DDRAM_ADRS;
	lcd_write_ins_nochk(temp);
	lcd_delay_40us();
}

/* THese delays are used instead of assuming a timer is available. */
void lcd_delay_40us()
{
	uint8_t count;
	
	for(count=0; count<LCD_DELAY_40US_COUNT; count++)
	{
		// Adjust for ~40us delay at 8MHz. Could be based on a CPU_CLOCK define. 4 clock loop.
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
	// output upper bits
	lcd_write_ins_once(data);
	// output lower bits
	lcd_write_ins_once(data<<4);
}

// Upper 4 bits are data.
void lcd_write_ins_once(uint8_t data)
{
	lcd_cmd_write(data);
}


/*
 * Uses two 4-bit reads.
 */
void lcd_busy_check()
{
	uint8_t busy = 0x80;
	while( busy != 0 )
	{
		busy = lcd_cmd_read();				// read LSBs
		busy = lcd_cmd_read() & 0x80;		// read MSBs
	}
}

/*
 * Only used for BUSY Flag check.
 */
uint8_t lcd_cmd_read()
{
	uint8_t temp;

	LCD_DISPLAY_RS_PORT &= ~(1<<LCD_DISPLAY_RS_PIN);		// RS=0
	LCD_DISPLAY_RW_PORT |= (1<<LCD_DISPLAY_RW_PIN);			// RW=1
	// Set up data
	LCD_DISPLAY_DATA_DDR &= 0x0F;							// set as input
	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 360ns width. 4 clocks @ 8MHz.
	asm("nop");
	asm("nop");
	asm("nop");												// 50ns clock at 20 MHz
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	temp = LCD_DISPLAY_DATA_PORTIN;							// read
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// clear E=0.

	return( temp );
}

/*
 * Diagram shows 360ns data setup max after rising edge of E.
 * Just setup data BEFORE raising E.
 * Hold time after E falls is 10ns. One nop will do.
 * ASSUMES lower four bits are data.
 */
void lcd_cmd_write(uint8_t data)
{
	uint8_t temp;
	
	LCD_DISPLAY_RS_PORT &= ~(1<<LCD_DISPLAY_RS_PIN);		// RS=0
	LCD_DISPLAY_RW_PORT &= ~(1<<LCD_DISPLAY_RW_PIN);		// RW=0
	// Set up data
	LCD_DISPLAY_DATA_DDR |= 0xF0;							// set as output..D4-D7
	temp = LCD_DISPLAY_DATA_PORT & 0x0F;					// clear bits first
	LCD_DISPLAY_DATA_PORT = temp | (data & 0xF0);			// then OR in data.
	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	asm("nop");												// 50ns clock at 20 MHz
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// clear E=0.
}

uint8_t lcd_ram_read()
{
	return 0;
}

/*
 * Diagram shows 360ns data setup max after rising edge of E.
 * Just setup data BEFORE raising E.
 * Hold time after E falls is 10ns. One nop will do.
 * Writes upper 4-bits as data.
 */
void lcd_ram_write(uint8_t data)
{
	uint8_t temp;
	
	LCD_DISPLAY_RS_PORT |= (1<<LCD_DISPLAY_RS_PIN);			// RS=1
	LCD_DISPLAY_RW_PORT &= ~(1<<LCD_DISPLAY_RW_PIN);		// RW=0
	// Set up data
	LCD_DISPLAY_DATA_DDR |= 0xF0;							// set as output..D4-D7
	temp = LCD_DISPLAY_DATA_PORT & 0x0F;					// clear bits first
	LCD_DISPLAY_DATA_PORT = temp | (data & 0xF0);			// then OR in data.
	// Toggle E
	LCD_DISPLAY_E_PORT |= (1<<LCD_DISPLAY_E_PIN);			// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	asm("nop");
	asm("nop");												// 50ns clock at 20 MHz
	asm("nop");
	asm("nop");
	asm("nop");
	LCD_DISPLAY_E_PORT &= ~(1<<LCD_DISPLAY_E_PIN);			// set E=0.
}
