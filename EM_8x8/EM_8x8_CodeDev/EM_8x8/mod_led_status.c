/*
 * mod_led_status.c
 *
 * Created: 8/13/2015	0.01	ndp
 *  Author: Chip
 *
 * This module controls the state of the nine Status LEDs.
 *
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "mod_led_status.h"
#include "sysTimer.h"
#include "access.h"


// Two single LEDs
#define MLS_D6_DDR		DDRD
#define MLS_D6_PORT		PORTD
#define MLS_D6_P		PD0

#define MLS_D7_DDR		DDRD
#define MLS_D7_PORT		PORTD
#define MLS_D7_P		PD2

// Seven differential LEDs on four lines
#define MLS_DIFF1_DDR	DDRD
#define MLS_DIFF1_PORT	PORTD
#define MLS_DIFF1_PA	PD1

#define MLS_DIFF2_DDR	DDRB
#define MLS_DIFF2_PORT	PORTB
#define MLS_DIFF2_PB	PB0
#define MLS_DIFF2_PC	PB1
#define MLS_DIFF2_PD	PB2


/* *** Local variables *** */
static uint8_t mod_led_status_state;




/*
 * Set up IO for Status LEDs
 */
void mod_led_status_init()
{
	mod_led_status_state = 0;
}

/*
 * Service Range Status LEDs D8:14
 * Cycle through each LED. If state bit is TRUE, turn ON for 1ms.
 *
 * NOTE: D6 & D7 are controlled by the bumper hardware.
 */
void mod_led_status_service()
{
	static uint8_t state;
	static uint8_t index = 8;
	
	if( GPIOR0 & (1<<DEV_1MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_1MS_TIC);

		// Reset temp for first LED.
		if( index == 8 )
		{
			state = mod_led_status_state;
		}

		if( state & 0x01 )
		{
			mls_turn_led_on(index);
		}
		else
		{
			mls_turn_led_off(index);
		}

		state >>= 1;				// adjust to test next bit.
		++index;
		if( index == 15 )
		{
			index = 8;			// reset index
		}
	}
}

/*
 * Manually turn ON LED.
 */
void mod_led_status_on()
{
	uint8_t led;
	
	led = getMsgData(3);		// get LED

	mls_turn_led_on( led );
	
	if( (led >= 8) && (led <= 14 ) )
	{
		mls_setRangeStatus( led-8, true );		// Update range status state register.
	}
}

/*
 * Manually turn OFF LED.
 */
void mod_led_status_off()
{
	uint8_t led;
	
	led = getMsgData(3);		// get LED

	mls_turn_led_off( led );
	
	if( (led >= 8) && (led <= 14 ) )
	{
		mls_setRangeStatus( led-8, false );		// Update range status state register.
	}
}

/*
 * Set range status LED state
 * 0:6 for D8:14
 */
void mls_setRangeStatus( uint8_t led, bool state )
{
	if(state)
	{
		mod_led_status_state |= (1 << led);
	}
	else
	{
		mod_led_status_state &= ~(1 << led);
	}
}

/*
 * The N value is the LED id on the schematic. (i.e. 6 is for D6)
 */
void mls_turn_led_on( uint8_t val )
{
	switch( val )
	{
		case 6:
			MLS_D6_DDR |= (1 << MLS_D6_P );			// first set as output
			MLS_D6_PORT &= ~(1 << MLS_D6_P );		// then set LOW to turn ON
			break;

		case 7:
			MLS_D7_DDR |= (1 << MLS_D7_P );			// first set as output
			MLS_D7_PORT &= ~(1 << MLS_D7_P );		// then set LOW to turn ON
			break;
			
		case 8:
			MLS_DIFF1_DDR |= (1 << MLS_DIFF1_PA );		// set active
			MLS_DIFF1_PORT &= ~(1 << MLS_DIFF1_PA );	// set LOW
		
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PB);	// set active
			MLS_DIFF2_PORT |= (1 << MLS_DIFF2_PB);	// set HIGH
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PC);	// set inactive
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PD);	// set inactive
			break;
		
		case 9:
			MLS_DIFF1_DDR |= (1 << MLS_DIFF1_PA );	// set active
			MLS_DIFF1_PORT |= (1 << MLS_DIFF1_PA );	// set HIGH
			
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PB);	// set active
			MLS_DIFF2_PORT &= ~(1 << MLS_DIFF2_PB);	// set LOW
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PC);	// set inactive
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PD);	// set inactive
			break;
		
		case 10:	// NEED TO FIX PCB FIRST.
			MLS_DIFF1_DDR |= (1 << MLS_DIFF1_PA );		// set active
			MLS_DIFF1_PORT &= ~(1 << MLS_DIFF1_PA );	// set LOW
		
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PB);	// set inactive
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PC);	// set active
			MLS_DIFF2_PORT |= (1 << MLS_DIFF2_PC);	// set HIGH
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PD);	// set inactive
			break;
		
		case 11:	// NEED TO FIX PCB FIRST.
			MLS_DIFF1_DDR |= (1 << MLS_DIFF1_PA );		// set active
			MLS_DIFF1_PORT |= (1 << MLS_DIFF1_PA );		// set HIGH
			
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PB);	// set inactive
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PC);	// set active
			MLS_DIFF2_PORT &= ~(1 << MLS_DIFF2_PC);	// set LOW
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PD);	// set inactive
			break;
		
		case 12:
			MLS_DIFF1_DDR &= ~(1 << MLS_DIFF1_PA );		// set inactive
		
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PB);	// set active
			MLS_DIFF2_PORT &= ~(1 << MLS_DIFF2_PB);	// set LOW
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PC);	// set active
			MLS_DIFF2_PORT |= (1 << MLS_DIFF2_PC);	// set HIGH
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PD);	// set inactive
			break;
		
		case 13:
			MLS_DIFF1_DDR &= ~(1 << MLS_DIFF1_PA );		// set inactive
		
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PB);	// set active
			MLS_DIFF2_PORT |= (1 << MLS_DIFF2_PB);	// set HIGH
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PC);	// set active
			MLS_DIFF2_PORT &= ~(1 << MLS_DIFF2_PC);	// set LOW
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PD);	// set inactive
			break;
		
		case 14:
			MLS_DIFF1_DDR &= ~(1 << MLS_DIFF1_PA );		// set inactive
		
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PB);	// set inactive
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PC);	// set active
			MLS_DIFF2_PORT &= ~(1 << MLS_DIFF2_PC);	// set LOW
			MLS_DIFF2_DDR |= (1 << MLS_DIFF2_PD);	// set active
			MLS_DIFF2_PORT |= (1 << MLS_DIFF2_PD);	// set HIGH
			break;
		
		default:	// ERROR
			break;
	}	
}

void mls_turn_led_off( uint8_t val )
{
	switch( val )
	{
		case 6:
			MLS_D6_DDR &= ~(1 << MLS_D6_P );		// first set as input
			MLS_D6_PORT |= (1 << MLS_D6_P );		// then set HIGH to turn OFF
			break;

		case 7:
			MLS_D7_DDR &= ~(1 << MLS_D7_P );		// first set as input
			MLS_D7_PORT |= (1 << MLS_D7_P );		// then set HIGH to turn OFF
			break;

		default:
			MLS_DIFF1_DDR &= ~(1 << MLS_DIFF1_PA );	// set inactive
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PB);	// set inactive
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PC);	// set inactive
			MLS_DIFF2_DDR &= ~(1 << MLS_DIFF2_PD);	// set inactive
			break;
	}
}
