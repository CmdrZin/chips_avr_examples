/*
 * io7SegLedsButtons.c
 *
 * Created: 11/25/2015		0.01	ndp
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "sysdefs.h"
#include "sysTimer.h"
#include "flash_table.h"
#include "io7SegLedsButtons.h"


#define IO_RESET_PORT		PORTC
#define IO_RESET_DDR		DDRC
#define IO_RESET_P			PORTC6

#define IO_BUTTON_UP_PORT	PORTB
#define IO_BUTTON_UP_DDR	DDRB
#define IO_BUTTON_UP_PIN	PINB
#define IO_BUTTON_UP_P		PORTB3

#define IO_BUTTON_DN_PORT	PORTB
#define IO_BUTTON_DN_DDR	DDRB
#define IO_BUTTON_DN_PIN	PINB
#define IO_BUTTON_DN_P		PORTB2

#define IO_SEG_PORT			PORTD
#define IO_SEG_DDR			DDRD

#define IO_LD_PORT			PORTB
#define IO_LD_DDR			DDRB
#define IO_LD_P				PORTB7

#define IO_DIG_1_PORT		PORTB
#define IO_DIG_1_DDR		DDRB
#define IO_DIG_1_P			PORTB0

#define IO_DIG_2_PORT		PORTB
#define IO_DIG_2_DDR		DDRB
#define IO_DIG_2_P			PORTB1

#define IDL_DIGIT_TIME		10		// N * 1ms

uint8_t idl_delay;				// duty cycle time for each LED.
IDL_DISPLAY idl_mode;

uint8_t idl_val1;				// DL1
uint8_t idl_val2;				// DL2

uint8_t idl_decimalVal;

/*
 * io_init()
 *
 * Initialize IO for LEDs and Buttons
 *
 */
void idl_init()
{
	IO_RESET_DDR &= ~(1<<IO_RESET_P);					// Set as INPUT
	IO_RESET_PORT |= (1<<IO_RESET_P);					// Enable internal Pull-Up
	
	IO_BUTTON_UP_DDR &= ~(1<<IO_BUTTON_UP_P);			// Set as INPUT
	IO_BUTTON_UP_PORT |= (1<<IO_BUTTON_UP_P);			// Enable internal Pull-Up
	
	IO_BUTTON_DN_DDR &= ~(1<<IO_BUTTON_DN_P);			// Set as INPUT
	IO_BUTTON_DN_PORT |= (1<<IO_BUTTON_DN_P);			// Enable internal Pull-Up

	IO_SEG_PORT = 0;									// Disable segments
	IO_SEG_DDR = 0xFF;									// set as output
	
	// Turn OFF both digits and Lpd
	IO_DIG_1_PORT &= ~(1<<IO_DIG_1_P);
	IO_DIG_2_PORT &= ~(1<<IO_DIG_2_P);
	IO_LD_DDR &= ~(1<<IO_LD_P);

	IO_DIG_1_DDR |= (1<<IO_DIG_1_P);					// set as outputs
	IO_DIG_2_DDR |= (1<<IO_DIG_2_P);					// set as outputs
	IO_LD_DDR |= (1<<IO_LD_P);							// set as outputs
	
	idl_delay = 1;				// N * 1 ms
	idl_mode = IDL_DL1;			// 0:OFF, 1:DL1, 2:DL2
	idl_val1 = 5;
	idl_val2 = 5;
	idl_decimalVal = 0;
}

/*
 * Service display
 */
void idl_service()
{
	if( GPIOR0 & (1<<DEV_1MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_1MS_TIC);				// clear flag
		if( --idl_delay == 0 )
		{
			idl_delay = IDL_DIGIT_TIME;

			switch( idl_mode )
			{
				case IDL_OFF:
					idl_displayDigit( 0, IDL_OFF );
					break;
				
				case IDL_DL1:
					idl_displayDigit( idl_val1, IDL_DL1 );
					idl_mode = IDL_DL2;
					break;
					
				case IDL_DL2:
					idl_displayDigit( idl_val2, IDL_DL2 );
					idl_mode = IDL_DL1;
					break;
					
				default:
					idl_mode = IDL_OFF;
					break;
			}
		}
	}
}

/*
 * Set display mode.
 */
void idl_setMode( IDL_DISPLAY mode )
{
	idl_mode = mode;
}


/*
 * Set display values.
 * val is two 8-bit values to support special characters.
 */
void idl_setValue( uint16_t val )
{
	idl_val1 = val>>8;
	idl_val2 = val & 0x00FF;
}

/* 
 * io_displayDigit()
 *
 * Display segments for value.
 * Value is 8-bit to support special characters.
 */
void idl_displayDigit( uint8_t val, IDL_DISPLAY digit )
{
	uint8_t	data;

	data = flash_get_seg_led( val );
	// Apply decimal point value

	// Turn OFF both digits
	IO_DIG_1_PORT &= ~(1<<IO_DIG_1_P);
	IO_DIG_2_PORT &= ~(1<<IO_DIG_2_P);
	IO_LD_PORT &= ~(1<<IO_LD_P);			// Turn OFF Ldp

	if( digit == IDL_DL1 )
	{
		// Apply decimal point value
		if( idl_decimalVal & 0x04 )
		{
			data |= 0x08;
		}
		if( idl_decimalVal & 0x08 )
		{
			IO_LD_PORT |= (1<<IO_LD_P);
		}
		// Turn ON digit
		IO_SEG_PORT = data;					// output segment pattern
		IO_DIG_1_PORT |= (1<<IO_DIG_1_P);
	}	
	
	if( digit == IDL_DL2 )
	{
		// Apply decimal point value
		if( idl_decimalVal & 0x01 )
		{
			data |= 0x08;
		}
		if( idl_decimalVal & 0x02 )
		{
			IO_LD_PORT |= (1<<IO_LD_P);
		}
		IO_SEG_PORT = data;					// output segment pattern
		IO_DIG_2_PORT |= (1<<IO_DIG_2_P);
	}
	
	// Else leave OFF
}

/*
 * Set decimal point states.
 * Use bit pattern of val. LRLR. 0:Off 1:On
 */
void idl_setDecimalPoints(uint8_t val)
{
	idl_decimalVal = val;
}

/*
 * Scan UP and DOWN buttons.
 * b1:UP, b0:DOWN	0:open, 1:closed
 */
uint8_t idl_scanButtons()
{
	uint8_t result = 0;
	
	if( !(IO_BUTTON_DN_PIN & (1<<IO_BUTTON_DN_P)) )		// test for GND (pressed)
		result |= 0x01;
		
	if( !(IO_BUTTON_UP_PIN & (1<<IO_BUTTON_UP_P)) )
	result |= 0x02;

	return( result );
}


/* *** LED Segment TABLE *** */
/*
 *      A
 *    F   B
 *      G
 *    E   C
 *   L  D  R
 *
 *    BGCDREFA
 */
const uint8_t ilb_segTable[] PROGMEM = {
	0b10110111,	//  0
	0b10100000,	//  1
	0b11010101,	//  2
	0b11110001,	//  3
	0b11100010,	//  4
	0b01110011,	//  5
	0b01110111,	//  6
	0b10100001,	//  7
	0b11110111,	//  8
	0b11100011,	//  9
	0b11100111,	//  A
	0b01110110,	//  B
	0b10110001,	//  C
	0b11110100,	//  D
	0b01010111,	//  E
	0b01000111,	//  F
	0b00000000,	//  blank
	0b01000000,	//  -
	0b01110100,	//  o
	0b00010110,	//  L
	0b01000100	//  r
};
