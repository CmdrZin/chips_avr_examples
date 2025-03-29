/*
 * lcd_hd44780.c
 *
 * Specific to HD44780U LCD controller w/ DMC24227_LCD display.
 * Minor adjustments may be needed to support other display modules.
 *
 * Created: 3/27/2025
 * Author: Chip
 *
 * Target: ATtiny1614
 */ 

#include <avr/io.h>
#include "lcd_hd44780.h"

#define HEADER  "HD44780->DMC24227 LCD"
#define VERSION "v 0.1"

// Local defines
#define LCD_DISPLAY_RS_DDR		PORTA.DIR
#define LCD_DISPLAY_RS_PORT_OUT PORTA.OUT
#define LCD_DISPLAY_RS_PIN		PIN4_bm

#define LCD_DISPLAY_RW_DDR		PORTA.DIR
#define LCD_DISPLAY_RW_PORT_OUT	PORTA.OUT
#define LCD_DISPLAY_RW_PIN		PIN5_bm

#define LCD_DISPLAY_E_DDR		PORTA.DIR
#define LCD_DISPLAY_E_PORT_OUT	PORTA.OUT
#define LCD_DISPLAY_E_PIN		PIN6_bm

#define LCD_DISPLAY_DDR        	PORTB.DIR
#define LCD_DISPLAY_PORT_IN     PORTB.IN
#define LCD_DISPLAY_MASK        0x0F
#define LCD_DISPLAY_PORT_OUT	PORTB.OUT

// Data Lines: D3:D0	D3:BUSY on first 4bit read.
// IR command		  i7:i4->D3:D0..i3:i0->D3:D0	/RS /RW  E+80ns->/E+10ns	Change data while /E
// Busy read		B,a6:a4->D3:D0..a3:a0->D3:D0	/RS  RW /E->E+160ns
// Data read		  d7:d4->D3:D0..d3:d0->D3:D0	/RS  RW /E->E+160ns
#define LCD_CLEAR_DISPLAY			0x01	// Write 0x20 to DDRAM and reset AC to 00. [1.52ms]
#define LCD_RETURN_HOME				0x02	// Reset AC to 00 and return cursor to home. [1.52ms]

#define LCD_SET_ENTRY_MODE			0x04	// b1: I/D, b0: S Set cursor move and display shift. [37us]
#define LCD_INCREMENT_ADRS	0x02
#define LCD_DECREMENT_ADRS	0x00
#define LCD_SHIFT_ENABLE	0x01
#define LCD_SHIFT_DISABLE	0x00

#define LCD_DISPLAY_ONOFF			0x08	// b2: Display, b1: Cursor, b0: Cursor position.  [37us]
#define LCD_DISPLAY_ON	0x04
#define LCD_DISPLAY_OFF	0x00
#define LCD_CURSOR_ON	0x02
#define LCD_CURSOR_OFF	0x00
#define LCD_BLINK_ON	0x01
#define LCD_BLINK_OFF	0x00

#define LCD_CURSOR_DISPLAY_SHIFT	0x10	// b3: Select, b2: R/L. [37us]

#define LCD_SET_FUNCTION			0x20	// b4: DL:1=8 bit, 0=4 bit, b3: N:1=2 lines, 0=1 line, b2: F:1=5x10, 0=5x8. [37us]
#define LCD_DL_8_BITS	0x10
#define LCD_DL_4_BITS	0x00
#define LCD_N_2_LINES	0x80
#define LCD_N_1_LINE	0x00
#define LCD_F_5X10		0x40
#define LCD_F_5X8		0x00

#define LCD_SET_CGRAM_ADRS			0x40	// b5:0 CGRAM Address. [37us]
#define LCD_SET_DDRAM_ADRS			0x80	// b6:0 DDRAM Address. [37us]

#define LCD_SHIFT_CURSOR_LEFT	0b00000000
#define LCD_SHIFT_CURSOR_RIGHT	0b00000100
#define LCD_SHIFT_DISPLAY_LEFT	0b00001000
#define LCD_SHIFT_DISPLAY_RIGHT	0b00001100

#define LCD_DELAY_40US_COUNT	34

/*
 * Initialize HD44780U for 4bit data. (pg 46)
 */
void lcd_init()
{
	// Set up IO. Control pins LOW.
	LCD_DISPLAY_RS_PORT_OUT &= ~(LCD_DISPLAY_RS_PIN);
	LCD_DISPLAY_RW_PORT_OUT &= ~(LCD_DISPLAY_RW_PIN);
	LCD_DISPLAY_E_PORT_OUT &= ~(LCD_DISPLAY_E_PIN);

	// Set up IO. Control pins are outputs.
	LCD_DISPLAY_RS_DDR |= LCD_DISPLAY_RS_PIN;
	LCD_DISPLAY_RW_DDR |= LCD_DISPLAY_RW_PIN;
	LCD_DISPLAY_E_DDR |= LCD_DISPLAY_E_PIN;

	// Data IO is set to input or output as needed.

	lcd_delay_1ms(50);			// wait 50ms after reset.
    lcd_writeCommand8NB(LCD_SET_FUNCTION | LCD_DL_8_BITS);		// set for 8bit
	lcd_delay_1ms(5);                                           // min 4.1 ms
    lcd_writeCommand8NB(LCD_SET_FUNCTION | LCD_DL_8_BITS);		// set for 8bit
	lcd_delay_1ms(1);                                           // min 100 us
    lcd_writeCommand8NB(LCD_SET_FUNCTION | LCD_DL_8_BITS);		// set for 8bit
	lcd_delay_40us();
    lcd_writeCommand8NB(LCD_SET_FUNCTION | LCD_DL_4_BITS);		// set for 4bit
	lcd_delay_40us();

	lcd_writeCommand(LCD_SET_FUNCTION | LCD_DL_4_BITS | LCD_N_2_LINES | LCD_F_5X8);
	lcd_delay_40us();
 	lcd_writeCommand(LCD_DISPLAY_ONOFF | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_OFF);
	lcd_delay_40us();
  	lcd_writeCommand(LCD_CLEAR_DISPLAY);
	lcd_delay_1ms(2);
  	lcd_writeCommand(LCD_SET_ENTRY_MODE | LCD_INCREMENT_ADRS);
	lcd_delay_40us();
    // Display address is set to 0x00 now.
	// Display Header
 
//    lcd_setDDadrs(1,5);
//	lcd_writeRam('C');
//	lcd_delay_40us();   
//	lcd_writeRam('A');
//	lcd_delay_40us();
    lcd_print(1,1,HEADER);
    lcd_print(2,4,VERSION);
}

// Print a NULL terminated string to the display at line 1:2, pos 0:23.
void lcd_print(uint8_t line, uint8_t pos, char* str)
{
    lcd_setDDadrs(line, pos);
    
    while(*str) {
        lcd_writeRam(*str);
        ++str;
    }
}

lcd_clearScreen()
{
    lcd_writeCommand(LCD_CLEAR_DISPLAY);
}

// Line = 1 or 2, pos = 0 to 23.
void lcd_setDDadrs( uint8_t line, uint8_t pos )
{
	uint8_t adrs;
    
    if( line == 1)
        adrs = 0x00;
    else
        adrs = 0x40;
    
    adrs += pos;
	
	adrs = adrs & 0x7F;			// mask data
	adrs |= LCD_SET_DDRAM_ADRS;
	lcd_writeCommand(adrs);
}

void lcd_setCGadrs( uint8_t adrs )
{
	uint8_t temp;
	
	temp = adrs & 0x3F;			// mask data
	temp |= LCD_SET_CGRAM_ADRS;
	lcd_writeCommand(temp);
}

/* These delays are used instead of assuming a timer is available. CPU is 3.33Mhz*/
void lcd_delay_40us()
{
	uint8_t count;
	
	for(count=0; count<LCD_DELAY_40US_COUNT; count++)
	{
		// Adjust for ~40us delay at 3.33MHz. Could be based on a CPU_CLOCK define. 4 clock loop.
		asm("NOP");
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

/*
 * Uses two 4-bit reads.
 * This is a BLOCKING command
 */
void lcd_busy_check()
{
	uint8_t temp;
	uint8_t busy = 0x08;
   
    // TODO: Set for MAX COUNT of 10 tries to prevent lock-up do to hardware failure.
	while( busy != 0 )
	{
        LCD_DISPLAY_RS_PORT_OUT &= ~(LCD_DISPLAY_RS_PIN);		// RS=0
    	LCD_DISPLAY_RW_PORT_OUT |= (LCD_DISPLAY_RW_PIN);		// RW=1
    	// Set up data read
    	LCD_DISPLAY_DDR &= ~(LCD_DISPLAY_MASK);			        // set as input..PB3:PB0
    	// Toggle E
    	LCD_DISPLAY_E_PORT_OUT |= LCD_DISPLAY_E_PIN;            // set E=1. Min 360ns width. 2 clocks @ 3.33MHz.
    	asm("NOP");
    	asm("NOP");
    	temp = LCD_DISPLAY_PORT_IN;                             // read
    	LCD_DISPLAY_E_PORT_OUT &= ~(LCD_DISPLAY_E_PIN);         // clear E=0.
		busy = temp & 0x08;                                     // read MSB  DB7:DB4 -> PB3:PB0
    	asm("NOP");                                             // small delay
    	// Toggle E for second nibble
    	LCD_DISPLAY_E_PORT_OUT |= LCD_DISPLAY_E_PIN;             // set E=1. Min 360ns width. 2 clocks @ 3.33MHz.
    	asm("NOP");
    	asm("NOP");
    	LCD_DISPLAY_E_PORT_OUT &= ~(LCD_DISPLAY_E_PIN);          // clear E=0.
	}
}

// Simple toggle of E line for data reads and writes.
void toggleE()
{
	// Toggle E
	LCD_DISPLAY_E_PORT_OUT |= LCD_DISPLAY_E_PIN;			// set E=1. Min 360ns width. 2 clocks @ 3.33MHz.
	asm("NOP");
	asm("NOP");
	LCD_DISPLAY_E_PORT_OUT &= ~(LCD_DISPLAY_E_PIN);		// clear E=0.
}

/*
 * Diagram shows 360ns data setup max after rising edge of E.
 * Just setup data BEFORE raising E.
 * Hold time after E falls is 10ns. One nop will do.
 */
void lcd_writeCommand(uint8_t cmd)
{
	LCD_DISPLAY_RS_PORT_OUT &= ~(LCD_DISPLAY_RS_PIN);		// RS=0
	LCD_DISPLAY_RW_PORT_OUT &= ~(LCD_DISPLAY_RW_PIN);		// RW=0
	// Set up data
	LCD_DISPLAY_DDR |= 0x0F;                                // set as output..PB3:PB0
	LCD_DISPLAY_PORT_OUT = (cmd >> 4);
	// Toggle E
	LCD_DISPLAY_E_PORT_OUT |= (LCD_DISPLAY_E_PIN);		// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	asm("NOP");
	asm("NOP");
	LCD_DISPLAY_E_PORT_OUT &= ~(LCD_DISPLAY_E_PIN);		// clear E=0.

	asm("NOP");                                             // small delay

	LCD_DISPLAY_PORT_OUT = (cmd & 0x0F);
	// Toggle E
	LCD_DISPLAY_E_PORT_OUT |= (LCD_DISPLAY_E_PIN);		// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	asm("NOP");
	asm("NOP");
	LCD_DISPLAY_E_PORT_OUT &= ~(LCD_DISPLAY_E_PIN);		// clear E=0.

    lcd_busy_check();                                       // verify that command has completed.
}

/*
 * Write 8-bit command with no BUSY check.
 * Used for initialization only.
 */
void lcd_writeCommand8NB(uint8_t cmd)
{
	LCD_DISPLAY_RS_PORT_OUT &= ~(LCD_DISPLAY_RS_PIN);		// RS=0
	LCD_DISPLAY_RW_PORT_OUT &= ~(LCD_DISPLAY_RW_PIN);		// RW=0
	// Set up data
	LCD_DISPLAY_DDR |= 0x0F;                                // set as output..PB3:PB0
	LCD_DISPLAY_PORT_OUT = (cmd >> 4);
	// Toggle E
	LCD_DISPLAY_E_PORT_OUT |= (LCD_DISPLAY_E_PIN);		// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	asm("NOP");
	LCD_DISPLAY_E_PORT_OUT &= ~(LCD_DISPLAY_E_PIN);		// clear E=0.
}

uint8_t lcd_readRam()
{
	return 0;
}

/*
 * Diagram shows 360ns data setup max after rising edge of E.
 * Just setup data BEFORE raising E.
 * Hold time after E falls is 10ns. One NPO will do.
 * Writes upper 4 bits then lower 4-bits to DD/CGRAM.
 */
void lcd_writeRam(uint8_t data)
{
	LCD_DISPLAY_RS_PORT_OUT |= (LCD_DISPLAY_RS_PIN);			// RS=1
	LCD_DISPLAY_RW_PORT_OUT &= ~(LCD_DISPLAY_RW_PIN);		// RW=0
	// Set up data
	LCD_DISPLAY_DDR |= 0x0F;							// set as output..PB3:PB0
	// Send upper bits
	LCD_DISPLAY_PORT_OUT = (data>>4);
	// Toggle E
	LCD_DISPLAY_E_PORT_OUT |= (LCD_DISPLAY_E_PIN);			// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	asm("NOP");
	LCD_DISPLAY_E_PORT_OUT &= ~(LCD_DISPLAY_E_PIN);			// set E=0.
	// Send lower bits
	LCD_DISPLAY_PORT_OUT = (data & 0x0F);			// then OR in data.
	// Toggle E
	LCD_DISPLAY_E_PORT_OUT |= (LCD_DISPLAY_E_PIN);			// set E=1. Min 240ns width. 2 clocks @ 8MHz.
	asm("NOP");
	LCD_DISPLAY_E_PORT_OUT &= ~(LCD_DISPLAY_E_PIN);			// set E=0.
	// Deactivate RS
	LCD_DISPLAY_RS_PORT_OUT &= ~(LCD_DISPLAY_RS_PIN);		// RS=0

   	lcd_delay_40us();   
}
