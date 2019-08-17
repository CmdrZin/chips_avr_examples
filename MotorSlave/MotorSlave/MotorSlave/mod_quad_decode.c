/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * mod_quad_decode.c
 *
 * Created: 4/18/2019 5:50:31 PM
 *  Author: Chip
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#define QUAD_M1_DDR		DDRB
#define QUAD_M2_DDR		DDRB
#define QUAD_M3_DDR		DDRC
#define QUAD_M4_DDR		DDRC

#define QUAD_M1_PORT	PORTB
#define QUAD_M2_PORT	PORTB
#define QUAD_M3_PORT	PORTC
#define QUAD_M4_PORT	PORTC

#define QUAD_M1_PINS	PINB
#define QUAD_M2_PINS	PINB
#define QUAD_M3_PINS	PINC
#define QUAD_M4_PINS	PINC

#define QUAD_M1_PIN_A	PB1		// PCINT9
#define QUAD_M1_PIN_B	PB2		// PCINT10
#define QUAD_M2_PIN_A	PB6		// PCINT14	
#define QUAD_M2_PIN_B	PB7		// PCINT15
#define QUAD_M3_PIN_A	PC3		// PCINT19
#define QUAD_M3_PIN_B	PC4		// PCINT20
#define QUAD_M4_PIN_A	PC6		// PCINT22
#define QUAD_M4_PIN_B	PC7		// PCINT23

// State machine defines
#define S_M1_00	((0<<QUAD_M1_PIN_A)|(0<<QUAD_M1_PIN_B))
#define S_M1_01	((0<<QUAD_M1_PIN_A)|(1<<QUAD_M1_PIN_B))
#define S_M1_10	((1<<QUAD_M1_PIN_A)|(0<<QUAD_M1_PIN_B))
#define S_M1_11	((1<<QUAD_M1_PIN_A)|(1<<QUAD_M1_PIN_B))

#define S_M2_00 ((0<<QUAD_M2_PIN_A)|(0<<QUAD_M2_PIN_B))
#define S_M2_01 ((0<<QUAD_M2_PIN_A)|(1<<QUAD_M2_PIN_B))
#define S_M2_10 ((1<<QUAD_M2_PIN_A)|(0<<QUAD_M2_PIN_B))
#define S_M2_11 ((1<<QUAD_M2_PIN_A)|(1<<QUAD_M2_PIN_B))

#define S_M3_00	((0<<QUAD_M3_PIN_A)|(0<<QUAD_M3_PIN_B))
#define S_M3_01	((0<<QUAD_M3_PIN_A)|(1<<QUAD_M3_PIN_B))
#define S_M3_10	((1<<QUAD_M3_PIN_A)|(0<<QUAD_M3_PIN_B))
#define S_M3_11	((1<<QUAD_M3_PIN_A)|(1<<QUAD_M3_PIN_B))

#define S_M4_00 ((0<<QUAD_M4_PIN_A)|(0<<QUAD_M4_PIN_B))
#define S_M4_01 ((0<<QUAD_M4_PIN_A)|(1<<QUAD_M4_PIN_B))
#define S_M4_10 ((1<<QUAD_M4_PIN_A)|(0<<QUAD_M4_PIN_B))
#define S_M4_11 ((1<<QUAD_M4_PIN_A)|(1<<QUAD_M4_PIN_B))

volatile static int position[4];			// current motor positions. Updated by interrupt service.

/*
 * Initialize Quadrature Decoder hardware and IO.
 */
void mqd_init()
{
	// Set up IO for input (default, may not be needed)
	QUAD_M1_DDR &= ~((1<<QUAD_M1_PIN_A)|(1<<QUAD_M1_PIN_B)|(1<<QUAD_M2_PIN_A)|(1<<QUAD_M2_PIN_B));		// M1 and M2 on same DDR
	QUAD_M3_DDR &= ~((1<<QUAD_M3_PIN_A)|(1<<QUAD_M3_PIN_B)|(1<<QUAD_M4_PIN_A)|(1<<QUAD_M4_PIN_B));		// M3 and M3 on same DDR
	// Set inputs with pull-ups
	QUAD_M1_PORT |= ((1<<QUAD_M1_PIN_A)|(1<<QUAD_M1_PIN_B)|(1<<QUAD_M2_PIN_A)|(1<<QUAD_M2_PIN_B));		// M1 and M2 on same PORT
	QUAD_M3_PORT |= ((1<<QUAD_M3_PIN_A)|(1<<QUAD_M3_PIN_B)|(1<<QUAD_M4_PIN_A)|(1<<QUAD_M4_PIN_B));		// M3 and M3 on same PORT
	// Set up for Level Change INTR
	PCMSK1 = (1<<PCINT9)|(1<<PCINT10)|(1<<PCINT14)|(1<<PCINT15);
	PCMSK2 = (1<<PCINT19)|(1<<PCINT20)|(1<<PCINT22)|(1<<PCINT23);
	PCICR = (1<<PCIF2)|(1<<PCIF1);

	for(int i=0; i<4; i++)
	{
		position[i] = 1000;		// ASSUME off homed position
	}
}

// Uses MotorIndex 0:3 for motors 1,2,3,4
int mqd_getPosition( uint8_t motorIndex)
{
	int val;

	cli();
	val = position[motorIndex];
	sei();

	return val;
}

// When against the mechanical stop, set position = HOME(0)
void mqd_setHome( uint8_t motorIndex )
{
	cli();
	position[motorIndex] = 0;		// HOMED
	sei();
}


// INTERRUPT SERVICE ROUTINES for Pin Level Change
/*
 * Motor	A	B		Pin Change Interrupt	Group
 *	1		PB1	PB2		PCINT9	PCINT10			PCMSK1
 *	2		PB6	PB7		PCINT14	PCINT15			PCMSK1
 *	3		PC3	PC4		PCINT19	PCINT20			PCMSK2
 *	4		PC6	PC7		PCINT22	OCINT23			PCMSK2
 *
 * FWD: 00, 01, 11, 10, 00, ...
 * REV: 00, 10, 11, 01, 00, ...
 */
ISR(PCINT1_vect)
{
	static uint8_t m1State = S_M1_00;			// pick a start point
	static uint8_t m2State = S_M2_00;			// pick a start point

	uint8_t data = QUAD_M1_PINS;		// Read PIN states
	uint8_t mQuad;
	
	mQuad = data & ((1<<QUAD_M1_PIN_A)|(1<<QUAD_M1_PIN_B));		// Mask out unused pins

	// Extra states only needed for better resolution.
	switch(m1State)
	{
		case S_M1_00:
			if(mQuad == S_M1_01) {
				// Forward
				++position[0];			// advance on this transition.
			} else {
				// Reverse				// treat invalid 11 as 10.
				--position[0];
			}
			break;

		case S_M1_01:
			break;

		case S_M1_11:
			break;

		case S_M1_10:
			break;
	}
	m1State = mQuad;						// Update to Next State

	mQuad = data & ((1<<QUAD_M2_PIN_A)|(1<<QUAD_M2_PIN_B));		// Mask out unused pins
	switch(m2State)
	{
		case S_M2_00:
			if(mQuad == S_M2_01) {
				// Forward
				++position[1];			// advance on this transition.
			} else {
				// Reverse				// treat invalid 11 as 10.
				--position[1];
			}
			break;

		case S_M2_01:
			break;

		case S_M2_11:
			break;

		case S_M2_10:
			break;
	}
	m2State = mQuad;						// Update to Next State
}

ISR(PCINT2_vect)
{
	static uint8_t m3State = 0;			// pick a start point
	static uint8_t m4State = 0;			// pick a start point

	uint8_t data = QUAD_M3_PINS;		// Read PIN states. 3 & 4 use the same Port.
	uint8_t mQuad;

	mQuad = data & ((1<<QUAD_M3_PIN_A)|(1<<QUAD_M3_PIN_B));		// Mask out unused pins

	switch(m3State)
	{
		case S_M3_00:
			if(mQuad == S_M3_01) {
				// Forward
				++position[2];			// advance on this transition.
			} else {
				// Reverse				// treat invalid 11 as 10.
				--position[2];
			}
			break;

		case S_M3_01:
			break;

		case S_M3_11:
			break;

		case S_M3_10:
			break;
	}
	m3State = mQuad;						// Update to Next State

	mQuad = data & ((1<<QUAD_M4_PIN_A)|(1<<QUAD_M4_PIN_B));		// Mask out unused pins
	switch(m4State)
	{
		case S_M4_00:
			if(mQuad == S_M4_01) {
				// Forward
				++position[3];			// advance on this transition.
			} else {
				// Reverse				// treat invalid 11 as 10.
				--position[3];
			}
			break;

		case S_M4_01:
			break;

		case S_M4_11:
			break;

		case S_M4_10:
			break;
	}
	m4State = mQuad;						// Update to Next State
}
