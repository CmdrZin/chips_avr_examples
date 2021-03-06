/*
 * ioLedsButtons.c
 *
 * Created: 8/25/2015 12:27:57 PM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "sysdefs.h"
#include "sysTimer.h"
#include "flash_table.h"
#include "ioLedsButtons.h"


#define IO_RESET_PORT			PORTC
#define IO_RESET_DDR			DDRC
#define IO_RESET_P				PORTC6

#define IO_BUTTON_MODE_PORT		PORTB
#define IO_BUTTON_MODE_DDR		DDRB
#define IO_BUTTON_MODE_PIN		PINB
#define IO_BUTTON_MODE_P		PORTB3

#define IO_BUTTON_OPTION_PORT	PORTB
#define IO_BUTTON_OPTION_DDR	DDRB
#define IO_BUTTON_OPTION_PIN	PINB
#define IO_BUTTON_OPTION_P		PORTB2

#define IO_DIFF_LEDS_PORT		PORTD
#define IO_DIFF_LEDS_DDR		DDRD
#define IO_DIFF_LEDS_MASK		( 0x07 )

#define IO_LED_PEAK_PORT		PORTB
#define IO_LED_PEAK_DDR			DDRB
#define IO_LED_PEAK_P			PORTB1				// Active LOW

#define IO_LED_POWER_PORT		PORTB
#define IO_LED_POWER_DDR		DDRB
#define IO_LED_POWER_P			PORTB0				// Active LOW

#define IO_DIFF_PD3				0x08				// Differential LED bit
#define IO_DIFF_PD4				0x10
#define IO_DIFF_PD5				0x20
#define IO_DIFF_PD6				0x40
#define IO_DIFF_PD7				0x80



uint8_t idl_delay;				// duty cycle time for each LED.
IO_DIFF_LED_MODE idl_mode;

uint8_t idl_dutyCount;
uint8_t idl_peakVal;
uint8_t idl_powerVal;
uint8_t idl_displayVal;			// update each full cycle.


/*
 * io_init()
 *
 * Initialize IO for LEDs and Buttons
 *
 */
void io_init()
{
	IO_RESET_DDR &= ~(1<<IO_RESET_P);					// Set as INPUT
	IO_RESET_PORT |= (1<<IO_RESET_P);					// Enable internal Pull-Up
	
	IO_BUTTON_MODE_DDR &= ~(1<<IO_BUTTON_MODE_P);		// Set as INPUT
	IO_BUTTON_MODE_PORT |= (1<<IO_BUTTON_MODE_P);		// Enable internal Pull-Up
	
	IO_BUTTON_OPTION_DDR &= ~(1<<IO_BUTTON_OPTION_P);	// Set as INPUT
	IO_BUTTON_OPTION_PORT |= (1<<IO_BUTTON_OPTION_P);	// Enable internal Pull-Up

	IO_DIFF_LEDS_DDR &= IO_DIFF_LEDS_MASK;				// Disable Differential LEDs

	IO_LED_PEAK_DDR |= (1<<IO_LED_PEAK_P);				// Set as OUTPUT
	IO_LED_PEAK_PORT |= (1<<IO_LED_PEAK_P);				// Turn OFF

	IO_LED_POWER_DDR |= (1<<IO_LED_POWER_P);			// Set as OUTPUT
	IO_LED_POWER_DDR |= (1<<IO_LED_POWER_P);			// Turn OFF
	
	idl_delay = 1;								// N * 10 ms
	idl_mode = IDLE;
	idl_peakVal = 0;
	idl_powerVal = 0;
	idl_dutyCount = 20;
}

/*
 * Select display type to service. Called each ms.
 */
void idl_service()
{
	if( GPIOR0 & (1<<DEV_1MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_1MS_TIC);				// clear flag
		
		switch( idl_mode )
		{
			case IDLE:
			break;
			
			case PEAK:
			ilb_servicePeak();
			break;
			
			case POWER:
			ilb_servicePower();
			break;
			
			default:
			idl_mode = IDLE;
			break;
		}
	}
}

void idl_setBarMode( IO_DIFF_LED_MODE mode )
{
	idl_mode = mode;

	// Restart display control if mode changes.
	idl_dutyCount = 20;
	if( mode == PEAK)
	{
		idl_displayVal = idl_peakVal;
	}
	else
	{
		idl_displayVal = idl_powerVal;
	}
}


void idl_setPeakValue( uint8_t val )
{
	idl_peakVal = val;
}

void idl_setPowerValue( uint8_t val )
{
	idl_powerVal = val;
}

/*
 * Display the PEAK value LED at 1/20 duty cycle to maintain the same brightness as the bar display.
 */
void ilb_servicePeak()
{
	if( idl_dutyCount == idl_displayVal )
	{
		io_turnDiffLedOn( idl_displayVal );
	}
	else
	{
		io_turnDiffLedOn(0);
	}

	if( --idl_dutyCount == 0 )
	{
		idl_dutyCount = 20;
		idl_displayVal = idl_peakVal;
	}
}

/*
 * Display the POWER value as a bar up to the LED at 1/20 duty cycle.
 */
void ilb_servicePower()
{
	if( idl_dutyCount <= idl_displayVal )
	{
		io_turnDiffLedOn( idl_dutyCount );
	}
	else
	{
		io_turnDiffLedOn(0);
	}
	
	if( --idl_dutyCount == 0 )
	{
		idl_dutyCount = 20;
		idl_displayVal = idl_powerVal;
	}
}


void io_setLedPeak( bool val )
{
	if( val )
	{
		IO_LED_PEAK_PORT &= ~(1<<IO_LED_PEAK_P);		// Turn ON
	}
	else
	{
		IO_LED_PEAK_PORT |= (1<<IO_LED_PEAK_P);			// Turn OFF
	}
}

void io_setLedPower( bool val )
{
	if( val )
	{
		IO_LED_POWER_PORT &= ~(1<<IO_LED_POWER_P);		// Turn ON
	}
	else
	{
		IO_LED_POWER_PORT |= (1<<IO_LED_POWER_P);		// Turn OFF
	}
}

/* 
 * io_turnDiffLedOn()
 *
 * Set IO for differential LED display.
 * led=0 to turn OFF all LEDs
 * NOTE: Only ONE LED can be on at a time.
 *
 */
void io_turnDiffLedOn( uint8_t led )
{
	ILB_DIFF_LED	data;
	
	data = flash_get_diff_led( led );
	
	IO_DIFF_LEDS_DDR &= IO_DIFF_LEDS_MASK;			// Clear old pattern
	IO_DIFF_LEDS_DDR |= data.ddr;					// set new pattern
	IO_DIFF_LEDS_PORT &= IO_DIFF_LEDS_MASK;
	IO_DIFF_LEDS_PORT |= data.p;
}


/* *** DIFF LED TABLE *** */
const ILB_DIFF_LED ilb_diffLedTable[] PROGMEM = {
	{ 0, 0 },											//  0 OFF
	{ IO_DIFF_PD3|IO_DIFF_PD7, IO_DIFF_PD3 },			//  1 D2
	{ IO_DIFF_PD3|IO_DIFF_PD7, IO_DIFF_PD7 },			//  2 D3
	{ IO_DIFF_PD3|IO_DIFF_PD6, IO_DIFF_PD3 },			//  3 D4
	{ IO_DIFF_PD3|IO_DIFF_PD6, IO_DIFF_PD6 },			//  4 D5
	{ IO_DIFF_PD4|IO_DIFF_PD7, IO_DIFF_PD4 },			//  5 D6
	{ IO_DIFF_PD4|IO_DIFF_PD7, IO_DIFF_PD7 },			//  6 D7
	{ IO_DIFF_PD4|IO_DIFF_PD6, IO_DIFF_PD4 },			//  7 D8
	{ IO_DIFF_PD4|IO_DIFF_PD6, IO_DIFF_PD6 },			//  8 D9
	{ IO_DIFF_PD5|IO_DIFF_PD7, IO_DIFF_PD5 },			//  9 D10
	{ IO_DIFF_PD5|IO_DIFF_PD7, IO_DIFF_PD7 },			// 10 D11
	{ IO_DIFF_PD3|IO_DIFF_PD5, IO_DIFF_PD3 },			// 11 D12
	{ IO_DIFF_PD3|IO_DIFF_PD5, IO_DIFF_PD5 },			// 12 D13
	{ IO_DIFF_PD6|IO_DIFF_PD7, IO_DIFF_PD6 },			// 13 D14
	{ IO_DIFF_PD6|IO_DIFF_PD7, IO_DIFF_PD7 },			// 14 D15
	{ IO_DIFF_PD5|IO_DIFF_PD6, IO_DIFF_PD5 },			// 15 D16
	{ IO_DIFF_PD5|IO_DIFF_PD6, IO_DIFF_PD6 },			// 16 D17
	{ IO_DIFF_PD4|IO_DIFF_PD5, IO_DIFF_PD4 },			// 17 D18
	{ IO_DIFF_PD4|IO_DIFF_PD5, IO_DIFF_PD5 },			// 18 D19
	{ IO_DIFF_PD3|IO_DIFF_PD4, IO_DIFF_PD3 },			// 19 D20
	{ IO_DIFF_PD3|IO_DIFF_PD4, IO_DIFF_PD4 }			// 20 D21
};
