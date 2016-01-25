/*
 * sysTimer.c
 *
 * System Timer Utility
 *
 * Created: 5/19/2015	0.01	ndp
 * Author: Chip
 *
 * revision:	01/19/2016	0.02	ndp		set to use 8MHz clock
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
// DEBUG
#include "mod_led_status.h"
// DEBUG --

#define SLOW_TIC		10			// 1ms * N for the slow tic

uint8_t	st_cnt_10ms;				// secondary timer counter.

volatile uint8_t st_tmr2_count;

/*
 * Set up Timer0 to generate System Time Tic for 1 ms using 8MHz CPU clock
 * Call this once after RESET.
 *
 * Modifies: OCR0A, TCCR0A, TIMSK0, TCCR0B, and GPIOR0
 *
 * input reg:	none
 * output reg:	none
 * resources:	R16
 *
 * NOTE: 1ms
 */
void st_init_tmr0()
{
	OCR0A = 61;		// 1ms = 8000000 / 8000 -> [2 * 64 * (1 + OCR0A)] : 128 * (62.5) -> OCR0A = 61
	
	TCCR0A = (1<<WGM01);

	TIMSK0 = (1<<OCIE0A);		// enable counter 0 OCO interrupt

	TCCR0B =  0b011;			// CPU div 64
	
	st_cnt_10ms = SLOW_TIC;

	GPIOR0 = 0;					// clear all tic flags

	return;
}

/*
 * Timer0 CTC (compare) interrupt service.
 * Called each 1ms
 *
 * input reg:	none
 * output reg:	none
 * resources:	GPIOR0.GPIR00:7
 * 				SRAM	1 byte
 *				Stack:3
 *
 */
ISR(TIMER0_COMPA_vect)
{
	// tic1ms flags
	GPIOR0 |= (1 << 0);
	GPIOR0 |= (1 << 1);
	GPIOR0 |= (1 << 2);
	GPIOR0 |= (1 << 3);

	if( --st_cnt_10ms == 0 )
	{
		GPIOR0 |= (1 << 4);
		GPIOR0 |= (1 << 5);
		GPIOR0 |= (1 << 6);
		GPIOR0 |= (1 << 7);
		st_cnt_10ms = SLOW_TIC;
	}
}

/*
 * Set up Timer2 to generate 56.4us interrupt for Sonar Time (1cm) using 20MHz CPU clock
 * Call this once after RESET.
 *
 * Modifies: OCR0A, TCCR0A, TIMSK0, TCCR0B, and GPIOR0
 *
 * input reg:	none
 * output reg:	none
 * resources:	R16
 *
 * NOTE: Rate set for 56.4us
 */
void st_init_tmr2()
{
	OCR2A = 140;

	TCCR2A = (1 << WGM01);		// reset on CTC match

	TCCR2B = 0x02;				// CPU div 8

	TIMSK2 =  (1<<OCIE2A);		// enable timer2 intr on A match.

	st_tmr2_clr();

	tmr2_clrCount();
}

uint8_t tmr2_getCount()
{
	return st_tmr2_count;
}

void tmr2_clrCount()
{
	st_tmr2_count = 0;
}

/*
 * Clear Timer2
 * Use for more accurate distance. Optional.
 */
void st_tmr2_clr()
{
	TCNT2 = 0;
}

/*
 * Timer2 CTC (compare) interrupt service.
 * Called each 56.4us to allow direct readout in cm for sonar.
 *
 * input reg:	none
 * output reg:	none
 * resources:	st_tmr2_count	SRAM	1 byte
 *
 * Limit Max count to 255 for NO rollover.
 *
 */
ISR( TIMER2_COMPA_vect )
{
	if( st_tmr2_count != 255 )
		++st_tmr2_count;
}
