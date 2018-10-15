/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Nels D. "Chip" Pearson (aka CmdrZin)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
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
 * mod_rcDecode.c
 *
 * Created: 4/23/2018 4:46:58 PM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "mod_rcDecode.h"
#include "mod_comms.h"

// DEBUG
#include "mod_led.h"

#define mc_STEERING	PINB4
#define mc_THROTTLE	PINB3

typedef enum {MRC_IDLE, MRC_MEASURE_ST, MRC_MEASURE_TT} MRC_STATES;

volatile static uint8_t stOVcnt;
volatile static uint8_t ttOVcnt;


void mod_rcDecode_init()
{
	// Initialize Timer2 for 4us clock with 8MHz clock. USE if too slow.
	TCCR2A = (0<<COM2A1)|(0<<COM2A0)|(0<<COM2B1)|(0<<COM2B0)|(0<<WGM21)|(0<<WGM20);	// Mode 0 (Normal)
	TCCR2B = (0<<WGM22)|(0<<CS22)|(1<<CS21)|(1<<CS20);		// div/32
	TIMSK2 = (1<<TOIE2);									// enable OV interrupt

	stOVcnt = 0;
	ttOVcnt = 0;
	
	// Set PB1 (PCINT1)and PB0 (PCINT0) to use Pin Change interrupts.
	DDRB &= ~((1<<mc_STEERING) | (1<<mc_THROTTLE));		// Set as INPUTS (default, may not be needed.)
	PCMSK0 = (1<<mc_STEERING) | (1<<mc_THROTTLE);
	PCICR = (1<<PCIE0);
}


// Need to test pin for level. Use state machine to service.
ISR(PCINT0_vect)
{
	bool pinST = false;
	bool pinTT = false;
	
	static MRC_STATES stateST = MRC_IDLE;
	static MRC_STATES stateTT = MRC_IDLE;
	
	static uint16_t stCount = 0;
	static uint16_t ttCount = 0;

	uint16_t timePulse;
	
	uint16_t count = TCNT2;					// capture count

	// Capture state of pins.
	if((PINB & (1<<mc_THROTTLE)) == (1<<mc_THROTTLE)) {
		pinTT = true;
	} else {
		pinTT = false;
	}
	if((PINB & (1<<mc_STEERING)) == (1<<mc_STEERING)) {
		pinST = true;
//		mod_led_on();
	} else {
		pinST = false;
//		mod_led_off();
	}
#if 1
	// Read tic4us
	//  a. Record time and set flag if pin is HIGH and flag is NOT set.
	//	b. Calculate time if pin LOW and flag is SET. Then clear flag.
	//  NOTE: 8 bit counter prone to overflow during timing. 256 * 4us = 1.024ms
	//  Use an OV flag for each channel. If set, set count to 250 (max).
	switch( stateST ) {
		case MRC_IDLE:
			if(pinST) {
				stCount = count;
				stOVcnt = 0;
				stateST = MRC_MEASURE_ST;
			}
			break;
			
		case MRC_MEASURE_ST:
			if(!pinST) {
				timePulse = stOVcnt;
				timePulse = timePulse<<8;
				timePulse += count;
				timePulse -= stCount;			// 00 --> 500 * 4us..0 --> 2ms
				if(timePulse < 250) {
					timePulse = 250;			// min 0
				}				
				timePulse -= 250;				// 00 --> 250
				if(timePulse > 250) {
					timePulse = 250;			// max 250
				}
				mod_comm_setReg( 4, (uint8_t)timePulse );			// STEERING
				stateST = MRC_IDLE;
			}
			// else not this pin.
			break;
			
		default:
			stateST = MRC_IDLE;
			break;		
	}		
	
	switch( stateTT ) {
		case MRC_IDLE:
			if(pinTT) {
				ttCount = count;
				ttOVcnt = 0;
				stateTT = MRC_MEASURE_TT;
			}
			break;
		
		case MRC_MEASURE_TT:
			if(!pinTT) {
				timePulse = ttOVcnt;
				timePulse = timePulse<<8;
				timePulse += count;
				timePulse -= ttCount;			// 00 --> 500 * 4us..0 --> 2ms
				if(timePulse < 250) {
					timePulse = 250;			// min 0
				}
				timePulse -= 250;				// 00 --> 250
				if(timePulse > 250) {
					timePulse = 250;			// max 250
				}
				mod_comm_setReg( 3, (uint8_t)timePulse );			// TROTTLE
				stateTT = MRC_IDLE;
			}
			// else not this pin.
			break;
		
		default:
			stateST = MRC_IDLE;
			break;
	}
#endif
}

ISR(TIMER2_OVF_vect)
{
	++stOVcnt;
	++ttOVcnt;
}
