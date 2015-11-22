/*
 * mod_range_finder_US.c
 *
 * Created: 8/16/2015		0.01	ndp
 *  Author: Chip
 *
 * This service supports three ultrasonic HR04 range sensors.
 *
 * 56.4us/cm .. Max range: 300cm .. Max wait: 17ms (use 20ms)
 *
 * Dependencies
 *   sysTimer
 *
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#include "mod_range_finder_US.h"
#include "sysTimer.h"
#include "access.h"
#include "mod_led_status.h"
#include "twiSlave.h"


#define MRFUS_ALL_DDR			DDRC
#define MRFUS_ALL_PIN			PINC
#define MRFUS_ALL_PORT			PORTC

#define	MRFUS_RIGHT_TRIG_P		PC2
#define	MRFUS_RIGHT_ECHO_P		PC3
#define	MRFUS_CENTER_TRIG_P		PC4
#define	MRFUS_CENTER_ECHO_P		PC5
#define	MRFUS_LEFT_TRIG_P		PC6
#define	MRFUS_LEFT_ECHO_P		PC7

#define MRFUS_IDLE_DELAY		1		// 50ms..Sensor idle scan
#define MRFUS_MAX_WAIT			5		// 50ms..Maximum ping wait delay 38ms

#define MRFUS_SEL_RIGHT			0		// N = Right sensor
#define MRFUS_SEL_CENTER		1		// N = Right sensor
#define MRFUS_SEL_LEFT			2		// N = Right sensor

#define MRFUS_RIGHT_LED			12-8	// D12
#define MRFUS_CENTER_LED		13-8	// D13
#define MRFUS_LEFT_LED			14-8	// D14

typedef enum { IDLE, LEFT, CENTER, RIGHT } MRFUS_RNG_SCAN;


// Current range registers
uint8_t mrfus_left;				// 0cm = 0..255cm = 0xFF
uint8_t mrfus_center;
uint8_t mrfus_right;

// Minimum range registers
uint8_t mrfus_left_min;			// 0cm = 0..255cm = 0xFF
uint8_t mrfus_center_min;
uint8_t mrfus_right_min;

MRFUS_RNG_SCAN mrfus_state;
uint8_t mrfus_delay;
uint8_t mrfus_transit;
uint8_t mrfus_pin;

/*
 * Initialize Sonar Range Sensor
 *
 * PCICR = (1<<PCIE2) .. Select PORTC edge detect group, other bit are 0.
 *
 */
void mod_range_finder_us_init()
{
	st_init_tmr2();			// Timer2 initialize for 1 count = 1cm
	
	MRFUS_ALL_DDR |= (1 << MRFUS_RIGHT_TRIG_P);		// output
	MRFUS_ALL_DDR &= ~(1 << MRFUS_RIGHT_ECHO_P);	// input
	
	MRFUS_ALL_DDR |= (1 << MRFUS_CENTER_TRIG_P);	// output
	MRFUS_ALL_DDR &= ~(1 << MRFUS_CENTER_ECHO_P);	// input

	MRFUS_ALL_DDR |= (1 << MRFUS_LEFT_TRIG_P);		// output
	MRFUS_ALL_DDR &= ~(1 << MRFUS_LEFT_ECHO_P);		// input

	PCICR = (1 << PCIE2);			// Select PORTC edge detect group

	mrfus_delay = MRFUS_IDLE_DELAY;
	mrfus_state = IDLE;
	
	mrfus_right_min = 20;			// Set minimum trigger range
	mrfus_center_min = 20;			// Set minimum trigger range
	mrfus_left_min = 20;			// Set minimum trigger range
}

/*
 * mod_range_finder_us_service()
 *
 * input reg:	none
 *
 * output reg:	none
 *
 * resources:	mrfus_left
 *				mrfus_center
 *				mrfus_right
 *
 * Three I/O lines for triggers.
 * Three I/O lines for interrupt response.
 * Cycle through sensors.
 *
 * Process
 * 0. Wait IDLE 50ms..Trigger Left..delay MAX
 * 1. Sample Left Detector..Trigger Center..delay MAX
 * 2. Sample Center Detector..Trigger Right..delay MAX
 * 3. Sample Right Detector..delay IDLE
 *
 */
void mod_range_finder_us_service()
{
	if( GPIOR0 & (1<<MRFUS_10MS_TIC) )
	{
		GPIOR0 &= ~(1<<MRFUS_10MS_TIC);				// clear flag
		
		if( --mrfus_delay == 0 )
		{
			switch( mrfus_state )
			{
				case IDLE:
					// Set up for LEFT scan
					MRFUS_ALL_PORT |= (1 << MRFUS_LEFT_TRIG_P);		// Set pin HIGH
					mrfus_trigger_wait();
					MRFUS_ALL_PORT &= ~(1 << MRFUS_LEFT_TRIG_P);	// Set pin LOW
					
					PCMSK2 = (1 << MRFUS_LEFT_ECHO_P);				// Enable pin change detect
					mrfus_pin = (1 << MRFUS_LEFT_ECHO_P);			// record pin to check.
					
					mrfus_delay = MRFUS_MAX_WAIT;
					
					mrfus_state = LEFT;
					break;
					
				case LEFT:
					mrfus_left = mrfus_transit;						// copy time of flight
					mls_setRangeStatus(MRFUS_LEFT_LED, (mrfus_left < mrfus_left_min) );
					
					// Set up for CENTER scan
					MRFUS_ALL_PORT |= (1 << MRFUS_CENTER_TRIG_P);	// Set pin HIGH
					mrfus_trigger_wait();
					MRFUS_ALL_PORT &= ~(1 << MRFUS_CENTER_TRIG_P);	// Set pin LOW
					
					PCMSK2 = (1 << MRFUS_CENTER_ECHO_P);			// Enable pin change detect
					mrfus_pin = (1 << MRFUS_CENTER_ECHO_P);			// record pin to check.
					
					mrfus_delay = MRFUS_MAX_WAIT;
					
					mrfus_state = CENTER;
					break;
					
				case CENTER:
					mrfus_center = mrfus_transit;						// copy time of flight
					mls_setRangeStatus(MRFUS_CENTER_LED, (mrfus_center < mrfus_center_min) );

					// Set up for RIGHT scan
					MRFUS_ALL_PORT |= (1 << MRFUS_RIGHT_TRIG_P);	// Set pin HIGH
					mrfus_trigger_wait();
					MRFUS_ALL_PORT &= ~(1 << MRFUS_RIGHT_TRIG_P);	// Set pin LOW
				
					PCMSK2 = (1 << MRFUS_RIGHT_ECHO_P);			// Enable pin change detect
					mrfus_pin = (1 << MRFUS_RIGHT_ECHO_P);			// record pin to check.
				
					mrfus_delay = MRFUS_MAX_WAIT;
				
					mrfus_state = RIGHT;
				break;

				case RIGHT:
					mrfus_right = mrfus_transit;						// copy time of flight
					mls_setRangeStatus(MRFUS_RIGHT_LED, (mrfus_right < mrfus_right_min) );
				
					mrfus_delay = MRFUS_IDLE_DELAY;
				
					mrfus_state = IDLE;
					break;

				default:
					mrfus_state = IDLE;
					break;
			}
		}
	}
}

/* Ping trigger delay 10us */
void mrfus_trigger_wait()
{
	for( uint8_t i=0; i<40; i++)
	{
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
	}
}

/*
 * Load range data into output FIFO
 * msgData[3] = N
 */
void mod_range_finder_us_getRange()
{
	twiTransmitByte( MOD_RANGE_FINDER_US_ID );
	
	switch( getMsgData(3) )
	{
		case MRFUS_SEL_RIGHT:
			twiTransmitByte( mrfus_right );
			break;

		case MRFUS_SEL_CENTER:
			twiTransmitByte( mrfus_center );
			break;

		case MRFUS_SEL_LEFT:
			twiTransmitByte( mrfus_left );
			break;
	}
}

/*
 * Load ALL range data into output FIFO
 */
void mod_range_finder_us_getAllRange()
{
	twiTransmitByte( MOD_RANGE_FINDER_US_ID );
	twiTransmitByte( mrfus_right );
	twiTransmitByte( mrfus_center );
	twiTransmitByte( mrfus_left );
}

/* Set minimum range to trigger LED. */
void mod_range_finder_us_setMinimumRange()
{
	twiTransmitByte( MOD_RANGE_FINDER_US_ID );
	
	switch( getMsgData(3) )
	{
		case MRFUS_SEL_RIGHT:
			mrfus_right_min = getMsgData(4);
			break;

		case MRFUS_SEL_CENTER:
			mrfus_center_min = getMsgData(4);
			break;

		case MRFUS_SEL_LEFT:
			mrfus_left_min = getMsgData(4);
			break;
	}
}

/* *** INTERUPT SERVICE *** */

/*
 * PCINT2 Change (PORTC pin change)
 * The monitored pin has changed.
 *
 * Check pin (range_s_pin)
 *   HIGH:	zero timer
 *   LOW:	copy timer and disable pin interrupt
 *
 * Both edges will be seen here. HIGH at start of time, LOW at end.
 */
ISR( PCINT2_vect )
{
	// Check active sensor pin state
	if( PINC & mrfus_pin )
	{
		// Clear count to 0 if still HIGH
		tmr2_clrCount();
	}
	else
	{
		// Copy captured range count.
		mrfus_transit = tmr2_getCount();
		PCMSK2 = 0;
	}
}
