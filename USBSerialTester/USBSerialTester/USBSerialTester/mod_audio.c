/*
* The MIT License (MIT)
*
* Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
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
* mod_audio.c
*
* Created:	9/24/2016		0.01	ndp
*  Author: Chip
* revision:	2/07/2017		0.02	ndp	update for ATmega328P, 1MHz
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

#include "sysTimer.h"
#include "mod_audio.h"
#include "io_led_button.h"				// DEBUG


// ATmega328P
#define SPEAKER_DDR		DDRB
#define SPEAKER_PORT	PORTB
#define SPEAKER_POUT	PORTB1				// OC1A

MA_ISR_SERVICE	isrMode;			// which type of waveform to play.
MA_SERVICE_TYPE	ma_serviceMode;

volatile bool		toneON;
bool		ma_playTune;
uint8_t		ma_step = 0;
uint16_t	accumStep;
uint16_t	accumulator;
uint16_t	pwmValue;
volatile uint16_t	ma_duration;				// N * ms
volatile uint8_t		ma_loopCount;
uint8_t		lastStep;
uint8_t		toneDelay;

uint16_t	ma_beepFreq;
uint8_t		ma_beepDuration;
uint8_t		ma_beepRepeat;

const char ma_sinTable112[];				// MUST be declared before use.
const uint16_t ma_tune01[];
const uint16_t ma_tune02[];
const uint16_t ma_tune03[];
const uint16_t ma_tune04[];
const uint16_t ma_tune05[];
const uint16_t ma_tune06[];
const uint16_t ma_tune07[];

void ma_init()
{
	// Set up Timer1. Set OC1A at BOTTOM:Clear on ICR1A Match, Fast PWM 8-bit(5), clk/1(001),
	//	TCCR1A |= (1<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (1<<WGM10);		// PWM(5)
	//	TCCR1A |= (0<<COM1A1) | (1<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);		// CTC(4)
	//	TCCR1B |= (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10);	// Clk/1
	//	TIMSK1 |= (1<<TOIE1);					// enable interrupt on overflow.
	OCR1A = 0x0080;
	ma_serviceMode = MA_SERVICE_NONE;
	isrMode = MA_ISR_SQUARE;

	ma_clearAudio();

	toneDelay = 10;					// N * 10ms delay.
}

/*
* ma_clearAudio()
* Reset audio flags, states, and other variables to stop tone/tune.
*/
void ma_clearAudio()
{
	// Clear variables
	ma_serviceMode = MA_SERVICE_NONE;
	ma_playTune = false;
	// Turn hardware OFF
	TCCR1A &= ~(1<<COM1A1);
	TCCR1A &= ~(1<<COM1A0);
	SPEAKER_DDR &= ~(1<<SPEAKER_POUT);		// set as input. Turn OFF.
	TIMSK1 = 0;								// disable interrupt on OCA match.
}

void ma_setBeepParam(uint16_t freq, uint8_t duration, uint8_t repetition)
{
	ma_beepFreq = freq;
	ma_beepDuration = duration;
	ma_beepRepeat = repetition;
//	if(ma_beepRepeat == 0) ++ma_beepRepeat;
	++ma_beepRepeat;							// needed due to countdown logic
}

/*
 * Set up Tone/Tune parameters for ma_service()
 */
void ma_setService(MA_SERVICE_TYPE val)
{
	switch(val)
	{
		case MA_SERVICE_NONE:
		ma_serviceMode = MA_SERVICE_NONE;
		TCCR1A &= ~(1<<COM1A1);					// Turn OFF.
		TCCR1A &= ~(1<<COM1A0);					// Turn OFF.
		SPEAKER_DDR &= ~(1<<SPEAKER_POUT);		// set as input. Turn OFF.
		TIMSK1 = 0;								// disable interrupt on overflow.
		break;

		// Configure Timer1 for square wave tone. All BEEPs serviced here.
		case MA_SERVICE_BEEP:
		ma_serviceMode = MA_SERVICE_BEEP;
		isrMode = MA_ISR_SQUARE;
		// Set up Timer1. Toggle OC1A on OCR1A Match, CTC(4), clk/1(001),
		TCCR1A = (0<<COM1A1) | (1<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);		// CTC(4)
		TCCR1B = (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (1<<CS11) | (0<<CS10);	// Clk/1
		toneDelay = 1;
		break;

		// Configure Timer1 for musical notes.
		case MA_SERVICE_SCORE1:
		case MA_SERVICE_SCORE2:
		case MA_SERVICE_SCORE3:
		case MA_SERVICE_SCORE4:
		case MA_SERVICE_SCORE5:
		case MA_SERVICE_SCORE6:
		case MA_SERVICE_SCORE7:
//		ma_serviceMode = MA_SERVICE_SCORE1;
		ma_serviceMode = val;
		isrMode = MA_ISR_SIN;
		// Set up Timer1. Set OC1A at BOTTOM:Clear on OCR1A Match, Fast PWM 8-bit(5), clk/1(001),
		TCCR1A = (1<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (1<<WGM10);
		TCCR1B = (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (1<<CS11) | (0<<CS10);	// Clk/1
		ma_step = 0;
		ma_playTune = true;
		break;

		default:
		ma_serviceMode = MA_SERVICE_NONE;
		TCCR1A &= ~(1<<COM1A1);					// Turn OFF.
		TCCR1A &= ~(1<<COM1A0);					// Turn OFF.
		SPEAKER_DDR &= ~(1<<SPEAKER_POUT);		// set as input. Turn OFF.
		TIMSK1 = 0;								// disable interrupt on overflow.
		break;
	}
}

/*
* Audio in process check.
* Return TRUE is there is a Tone/Tune playing.
* Used to wait until Tone/Tune completes.
*/
bool ma_isPlaying()
{
//	return (ma_serviceMode != MA_SERVICE_NONE);
	if(ma_serviceMode == MA_SERVICE_NONE)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ma_service()
{
	static uint8_t notes;
	static uint16_t* noteAdrs;
	static long lastTime = 10L;

	uint16_t note;
	uint16_t	dur;

	switch(ma_serviceMode)
	{
		case MA_SERVICE_NONE:
		break;

		case MA_SERVICE_BEEP:
		if (lastTime < st_millis())
		{
			lastTime += 10;
			if(--toneDelay == 0)
			{
				toneDelay = 50;					// N * 10ms delay.
//				ma_tone(ma_beepFreq, ma_beepDuration);
				if(--ma_beepRepeat == 0)
				{
					ma_serviceMode = MA_SERVICE_NONE;
				}
				else
				{
					ma_tone(ma_beepFreq, ma_beepDuration);
				}
			}
		}
		break;

		case MA_SERVICE_SCORE1:
		if(ma_playTune && !toneON)
		{
			if(ma_step == 0) {
				noteAdrs = ma_tune05;
				notes = pgm_read_word(noteAdrs);
				++noteAdrs;
			}
			note = pgm_read_word(noteAdrs);
			++noteAdrs;
			dur = pgm_read_word(noteAdrs);
			++noteAdrs;

			ma_tone(note, dur);

			if(++ma_step >= notes) {
				ma_step = 0;
				ma_playTune = false;
				ma_serviceMode = MA_SERVICE_NONE;
			}
		}
		break;

		case MA_SERVICE_SCORE2:
		if(ma_playTune && !toneON)
		{
			if(ma_step == 0) {
				noteAdrs = ma_tune01;
				notes = pgm_read_word(noteAdrs);
				++noteAdrs;
			}
			note = pgm_read_word(noteAdrs);
			++noteAdrs;
			dur = pgm_read_word(noteAdrs);
			++noteAdrs;

			ma_tone(note, dur);

			if(++ma_step >= notes) {
				ma_step = 0;
				ma_playTune = false;
				ma_serviceMode = MA_SERVICE_NONE;
			}
		}
		break;

		case MA_SERVICE_SCORE3:
		if(ma_playTune && !toneON)
		{
			if(ma_step == 0) {
				noteAdrs = ma_tune02;
				notes = pgm_read_word(noteAdrs);
				++noteAdrs;
			}
			note = pgm_read_word(noteAdrs);
			++noteAdrs;
			dur = pgm_read_word(noteAdrs);
			++noteAdrs;

			ma_tone(note, dur);

			if(++ma_step >= notes) {
				ma_step = 0;
				ma_playTune = false;
				ma_serviceMode = MA_SERVICE_NONE;
			}
		}
		break;

		case MA_SERVICE_SCORE4:
		if(ma_playTune && !toneON)
		{
			if(ma_step == 0) {
				noteAdrs = ma_tune03;
				notes = pgm_read_word(noteAdrs);
				++noteAdrs;
			}
			note = pgm_read_word(noteAdrs);
			++noteAdrs;
			dur = pgm_read_word(noteAdrs);
			++noteAdrs;

			ma_tone(note, dur);

			if(++ma_step >= notes) {
				ma_step = 0;
				ma_playTune = false;
				ma_serviceMode = MA_SERVICE_NONE;
			}
		}
		break;

		case MA_SERVICE_SCORE5:
		if(ma_playTune && !toneON)
		{
			if(ma_step == 0) {
				noteAdrs = ma_tune04;
				notes = pgm_read_word(noteAdrs);
				++noteAdrs;
			}
			note = pgm_read_word(noteAdrs);
			++noteAdrs;
			dur = pgm_read_word(noteAdrs);
			++noteAdrs;

			ma_tone(note, dur);

			if(++ma_step >= notes) {
				ma_step = 0;
				ma_playTune = false;
				ma_serviceMode = MA_SERVICE_NONE;
			}
		}
		break;

		case MA_SERVICE_SCORE6:
		if(ma_playTune && !toneON)
		{
			if(ma_step == 0) {
				noteAdrs = ma_tune06;
				notes = pgm_read_word(noteAdrs);
				++noteAdrs;
			}
			note = pgm_read_word(noteAdrs);
			++noteAdrs;
			dur = pgm_read_word(noteAdrs);
			++noteAdrs;

			ma_tone(note, dur);

			if(++ma_step >= notes) {
				ma_step = 0;
				ma_playTune = false;
				ma_serviceMode = MA_SERVICE_NONE;
			}
		}
		break;

		case MA_SERVICE_SCORE7:
		if(ma_playTune && !toneON)
		{
			if(ma_step == 0) {
				noteAdrs = ma_tune07;
				notes = pgm_read_word(noteAdrs);
				++noteAdrs;
			}
			note = pgm_read_word(noteAdrs);
			++noteAdrs;
			dur = pgm_read_word(noteAdrs);
			++noteAdrs;

			ma_tone(note, dur);

			if(++ma_step >= notes) {
				ma_step = 0;
				ma_playTune = false;
				ma_serviceMode = MA_SERVICE_NONE;
			}
		}
		break;

		default:
		ma_serviceMode = MA_SERVICE_NONE;
		ma_step = 0;
		ma_playTune = false;
		break;
	}
}

// trigger wave tone.
void ma_tone( uint16_t freq, uint16_t duration )
{
	accumStep = freq;
	ma_duration = duration;
	ma_loopCount = 4;						// initialize loop count.
	toneON = true;

	if(freq == 0) {
		// No sound pause.
		SPEAKER_DDR &= ~(1<<SPEAKER_POUT);		// set as input. Turn OFF.
		TCCR1A &= ~(1<<COM1A1);					// set as input. Turn OFF.
		TCCR1A &= ~(1<<COM1A0);					// set as input. Turn OFF.
	} else {
		// Set up for Note or Beep.
		SPEAKER_DDR |= (1<<SPEAKER_POUT);		// set as output. Turn ON.
		if(isrMode == MA_ISR_SIN) {
			TCCR1A |= (1<<COM1A1);				// set for PWM output. Turn ON.
			TCCR1A &= ~(1<<COM1A0);
		} else {
			OCR1A = freq;
			TCCR1A &= ~(1<<COM1A1);				// set for SQUARE output. Turn ON.
			TCCR1A |= (1<<COM1A0);
		}
	}

	if(isrMode == MA_ISR_SIN) {
		TIMSK1 |= (1<<TOIE1);				// enable interrupt on overflow.
	} else {
		TIMSK1 |= (1<<OCIE1A);				// enable interrupt on OCA match.
	}
}

/*
* Output Beep for Duration. Clear toneON flag when done.
*/
ISR(TIMER1_COMPA_vect)
{
	if (--ma_loopCount == 0)
	{
		if (--ma_duration == 0)
		{
			toneON = false;
			TCCR1A &= ~(1<<COM1A1);		// Turn OFF.
			TCCR1A &= ~(1<<COM1A0);
			SPEAKER_DDR &= ~(1<<SPEAKER_POUT);		// set as input. Turn OFF.
			TIMSK1 = 0;								// disable interrupt on OCA match.
		} else {
			ma_loopCount = 4;
		}
	}
}

/*
* Output Tone for Duration. Clear toneON flag when done.
*/
ISR(TIMER1_OVF_vect)
{
	OCR1A = pwmValue;
	accumulator += accumStep;

	switch(isrMode)
	{
		case MA_ISR_SIN:
		pwmValue = (uint16_t)(ma_getSinVal(accumulator>>8) + 128);		// use upper 8 bits for table index.
		break;

		case MA_ISR_SQUARE:
		break;

		default:
		OCR1A = 0x0080;
		break;
	}

	if (--ma_loopCount == 0)
	{
		if (--ma_duration == 0)
		{
			toneON = false;
			TCCR1A &= ~(1<<COM1A1);		// Turn OFF.
			TCCR1A &= ~(1<<COM1A0);
			SPEAKER_DDR &= ~(1<<SPEAKER_POUT);		// set as input. Turn OFF.
			TIMSK1 = 0;								// disable interrupt on overflow.
		} else {
			ma_loopCount = 4;
		}
	}
}

/*
* 256 entry sin() table
*/
char ma_getSinVal( uint8_t index )
{
	return( pgm_read_byte(&ma_sinTable112[index]) );
}

// Notes MA_C3 -> MA_B6 are available.

// (9)
const uint16_t ma_tune01[] PROGMEM = {
	10,
	MA_A5, 250, 0, 100, MA_D5, 250, 0, 100, MA_B5, 250, 0, 100, MA_G5, 200, 0, 50, MA_A5, 250, 0, 250
};

// (6) Jingle Bells
const uint16_t ma_tune02[] PROGMEM = {
	24,
	MA_B5, 125, 0, 100, MA_B5, 125, 0, 100, MA_B5, 250, 0, 100, MA_B5, 125, 0, 100, MA_B5, 125, 0, 100, MA_B5, 250, 0, 100,
	MA_B5, 125, 0, 100, MA_D6, 125, 0, 100, MA_G5, 125, 0, 100, MA_A5, 125, 0, 100, MA_B5, 250, 0, 250, 0, 250, 0, 250    
};

// (7)
const uint16_t ma_tune03[] PROGMEM = {
	41,
	MA_E5, 125, 0, 100, MA_D5, 125, 0, 100, MA_G5, 125, 0, 100, MA_A5, 125, 0, 100, MA_E5, 125, 0, 100,
	MA_D5, 125, 0, 100, MA_C5, 250, 0, 100, MA_B4, 125, 0, 100, MA_B4, 62, 0, 100, MA_B4, 62, 0, 100,
	MA_D5, 125, 0, 100, MA_F5, 125, 0, 100, MA_A5, 125, 0, 100, MA_D6, 125, 0, 100, MA_C6, 250, 0, 100,
	MA_B5, 250, 0, 100, MA_A5, 125, MA_A5, 250, 0, 100, MA_B5, 125, 0, 100, MA_C6, 250, 0, 250, 0, 250, 0, 250
};

// (8)
const uint16_t ma_tune04[] PROGMEM = {
	41,
	MA_E4, 125, 0, 100, MA_D4, 125, 0, 100, MA_G4, 125, 0, 100, MA_A4, 125, 0, 100, MA_E4, 125, 0, 100,
	MA_D4, 125, 0, 100, MA_C4, 250, 0, 100, MA_B4, 125, 0, 100, MA_B3, 62, 0, 100, MA_B3, 62, 0, 100,
	MA_D4, 125, 0, 100, MA_F4, 125, 0, 100, MA_A4, 125, 0, 100, MA_D5, 125, 0, 100, MA_C5, 250, 0, 100,
	MA_B4, 250, 0, 100, MA_A4, 125, MA_A4, 250, 0, 100, MA_B4, 125, 0, 100, MA_C5, 250, 0, 250, 0, 250, 0, 250
};

// (5) Old ACK
const uint16_t ma_tune05[] PROGMEM = {
	8,
	MA_A5, 250, 0, 100, MA_C5, 250, 0, 100, MA_G5, 250, 0, 100, MA_C5, 250, 0, 250
};

// (3)
const uint16_t ma_tune06[] PROGMEM = {
	13,
	MA_C5, 250, 0, 100, MA_E5, 125, 0, 100, MA_E5, 62, 0, 100, MA_E5, 62, 0, 100, MA_G5, 250, MA_G5, 250, 
	0, 250, 0, 250, 0, 250
};

// (4)
const uint16_t ma_tune07[] PROGMEM = {
	28,
	MA_C5, 125, 0, 100, MA_C5, 125, 0, 100, MA_G5, 250, 0, 100, MA_A5, 125, 0, 100, MA_F5, 125, 0, 100,
	MA_G5, 250, 0, 100, MA_A5, 125, 0, 100, MA_B5, 125, 0, 100, MA_C6, 125, 0, 100, MA_A5, 125, 0, 100,
	MA_G5, 125, 0, 100, MA_D5, 125, 0, 100, MA_C5, 250, 0, 250, 0, 250, 0, 250
};

const char ma_sinTable112[] PROGMEM = {
	0,   3,   5,   8,  11,  14,  16,  19,  22,  25,  27,  30,  33,  35,  38,  40,
	43,  45,  48,  50,  53,  55,  58,  60,  62,  64,  67,  69,  71,  73,  75,  77,
	79,  81,  83,  85,  87,  88,  90,  92,  93,  95,  96,  97,  99, 100, 101, 102,
	103, 104, 105, 106, 107, 108, 109, 109, 110, 110, 111, 111, 111, 112, 112, 112,
	112, 112, 112, 112, 111, 111, 111, 110, 110, 109, 109, 108, 107, 106, 105, 104,
	103, 102, 101, 100,  99,  97,  96,  95,  93,  92,  90,  88,  87,  85,  83,  81,
	79,  77,  75,  73,  71,  69,  67,  64,  62,  60,  58,  55,  53,  50,  48,  45,
	43,  40,  38,  35,  33,  30,  27,  25,  22,  19,  16,  14,  11,   8,   5,   3,
	0,  -3,  -5,  -8, -11, -14, -16, -19, -22, -25, -27, -30, -33, -35, -38, -40,
	-43, -45, -48, -50, -53, -55, -58, -60, -62, -64, -67, -69, -71, -73, -75, -77,
	-79, -81, -83, -85, -87, -88, -90, -92, -93, -95, -96, -97, -99,-100,-101,-102,
	-103,-104,-105,-106,-107,-108,-109,-109,-110,-110,-111,-111,-111,-112,-112,-112,
	-112,-112,-112,-112,-111,-111,-111,-110,-110,-109,-109,-108,-107,-106,-105,-104,
	-103,-102,-101,-100, -99, -97, -96, -95, -93, -92, -90, -88, -87, -85, -83, -81,
	-79, -77, -75, -73, -71, -69, -67, -64, -62, -60, -58, -55, -53, -50, -48, -45,
	-43, -40, -38, -35, -33, -30, -27, -25, -22, -19, -16, -14, -11,  -8,  -5, -3
};
