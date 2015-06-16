/*
 * sysTimer.c
 *
 * System Timer Utility
 *
 * Created: 5/19/2015 12:26:01 PM
 * Author: Chip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"

#define SLOW_TIC		10			// 1ms * N for the slow tic

uint8_t	st_cnt_10ms;				// secondary timer counter.

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
//	ldi		R16, 31				; 2 * 256 * (1 + OCR0A) : 512 * (16*2) : 1024 * 8 * 2
//	out		OCR0A, R16
	OCR0A = 31;					// 2 * 256 * (1 + OCR0A) : 512 * (16*2) : 1024 * 8 * 2

//	ldi		R16, (1<<WGM01)
//	out		TCCR0A, R16
	TCCR0A = (1<<WGM01);

//	ldi		R16, (1<<OCIE0A)
//	sts		TIMSK0, R16			; enable counter 0 OCO intr
	TIMSK0 = (1<<OCIE0A);		// enable counter 0 OCO interrupt

//	ldi		R16, 0b100			; CPU div 256
//	out		TCCR0B, R16
	TCCR0B =  0b100;			// CPU div 256

//	ldi		R16, SLOW_TIC		; set for 10ms count.
//	sts		st_cnt_10ms, R16
	st_cnt_10ms = SLOW_TIC;

//	clr		R16
//	sts		GPIOR0, R16			; clear all tic flags
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
ISR(TIM0_COMPA_vect)
{
	// tic1ms flags
// sbi		GPIOR0, GPIOR00		;
// sbi		GPIOR0, GPIOR01		;
// sbi		GPIOR0, GPIOR02		;
// sbi		GPIOR0, GPIOR03		;
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
