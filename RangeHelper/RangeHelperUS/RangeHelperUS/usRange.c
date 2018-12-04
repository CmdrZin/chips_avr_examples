/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * usRange.c
 *
 * Created: 12/1/2018 12:45:40 PM
 *  Author: Chip
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
#include "usRange.h"

#define ECHO_PORT		PORTB
#define ECHO_DDR		DDRB
#define ECHO_PIN		PB3
#define ECHO_INPUT		PINB

#define TRIGGER_PORT	PORTB
#define TRIGGER_DDR		DDRB
#define TRIGGER_PIN		PB4

typedef enum usrState {USR_IDLE, USR_WAIT} USR_STATE;

static uint8_t 	usr_tmr1Count;
static uint8_t	usr_range;


void usr_init()
{
	ECHO_DDR &= ~(1<<ECHO_PIN);			// set as input.
	TRIGGER_DDR |= (1<<TRIGGER_PIN);		// set as output.
	TRIGGER_PORT &= ~(1<<TRIGGER_PIN);		// set LOW.

	usr_tmr1Init();	
}

/*
 * Set up Timer1 to generate 56.4us interrupt for Sonar Time (1cm) using 8MHz CPU clock
 * Call this once after RESET.
 *
 * NOTE: Rate set for 56.4us
 */
void usr_tmr1Init()
{
	OCR1A = 25;							// 55 OCR1A causes an interrupt.
	OCR1C = 26;							// 56 OCR1C causes the timer to reset to $00. Does NOT cause an interrupt.

	TCCR1 = (1<<CTC1)|(1<<CS12);		// reset on CTC match to OCR1C, CPU div 8.
	
	GTCCR = 0;

	TIMSK |= (1<<OCIE1A);				// enable timer1 interrupt on OCR1A match.

	usr_tmr1ClrCount();
}

uint8_t usr_tmr1GetCount()
{
	return usr_tmr1Count;
}

void usr_tmr1ClrCount()
{
	usr_tmr1Count = 0;
}

/* Return range in cm. */
uint8_t usr_getRange()
{
	return usr_range;
}

/*
 * Scan PB3 for Echo return. Trigger PB4 for 10us trigger pulse.
 * Allow 50ms max return time.
 */
uint8_t usr_service()
{
	static USR_STATE state = USR_IDLE;
	static uint32_t lastTrigger = 0;

	uint8_t status = 0;
	
	switch(state) {
		case USR_IDLE:
			if( lastTrigger < st_millis() ) {
				lastTrigger += 50;
				usr_trigger();
				state = USR_WAIT;
				// Wait for ECHO for go HIGH.
				while( (ECHO_INPUT & (1<<ECHO_PIN)) == 0 ) {
					;
				}
				usr_tmr1ClrCount();
			}
			break;
			
		case USR_WAIT:
			// Check for ECHO
			if( (ECHO_INPUT & (1<<ECHO_PIN)) == 0 ) {
				usr_range = usr_tmr1GetCount();
				state = USR_IDLE;
				status = 1;
			}
			break;
			
		default:
			state = USR_IDLE;
			break;
	} 	
	
	return status;
}

/* Generate 10us pulse to trigger ranger. 
 * 8 MHz clock = 0.125us clock
 */
void usr_trigger()
{
	TRIGGER_PORT |= (1<<TRIGGER_PIN);

	for(int i=0; i<20; i++) {
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
	}		

	TRIGGER_PORT &= ~(1<<TRIGGER_PIN);
}

/* *** INTERUPT SERVICES *** */
/*
 * Timer1 CompA (compare) interrupt service.
 * Called each 56.4us to allow direct readout in cm for sonar.
 *
 * Limit Max count to 255 for NO rollover.
 */
ISR( TIMER1_COMPA_vect )
{
	if( usr_tmr1Count != 255 )
		++usr_tmr1Count;
}
