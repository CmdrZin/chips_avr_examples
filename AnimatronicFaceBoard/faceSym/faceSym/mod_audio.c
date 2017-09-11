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
* Created: 9/24/2016 10:14:30 PM
*  Author: Chip
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

#include "sysTimer.h"
#include "mod_audio.h"

#define SPEAKER_DDR		DDRD
#define SPEAKER_PORT	PORTD
#define SPEAKER_POUT	PD5				// OC1A

bool		toneON;
uint16_t	accumStep;
uint16_t	accumulator;
uint16_t	pwmValue;
uint16_t	ma_duration;				// N * ms
uint8_t		ma_loopCount;
uint8_t		lastStep;

uint8_t		toneDelay;

const char ma_sinTable[];				// MUST be declared before use.
const char ma_sinTable112[];				// MUST be declared before use.

void ma_init()
{
	// Set up Timer1. Set OC1A at BOTTOM:Clear on ICR1A Match, Fast PWM 8-bit(5), clk/1(001),
	TCCR1A |= (1<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (1<<WGM10);
	TCCR1B |= (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10);	// Clk/1
	TIMSK1 |= (1<<TOIE1);					// enable interrupt on overflow.

	OCR1A = 0x0060;

	// Set I/O for output
	SPEAKER_DDR |= (1<<SPEAKER_POUT);	// set as output.

	toneDelay = 100;					// N * 10ms delay.
}


void ma_service()
{
	static uint8_t ma_step = 0;

	uint16_t note;
	#if 0
	if (GPIOR0 & (1<<AUDIO_10MS_TIC))
	{
		GPIOR0 &= ~(1<<AUDIO_10MS_TIC);			// clear flag

		if(--toneDelay == 0)
		{
			toneDelay = 100;					// N * 10ms delay.
			ma_tone((800), 500);				// ~1kHz
			//			ma_tone((32*256), 3500);			// ~10kHz top limit
		}
	}
	#else
	#if 0
	uint16_t	dur;
	static bool	pause = false;
	if(!toneON)
	{
		if(pause)
		{
			note = 0;
			dur = 200;
			pause = false;
		}
		else
		{
			switch(ma_step)
			{
				case 0:
				note = MA_1000*4;
				dur = 20;
				break;

				case 1:
				note = MA_1000*4;
				dur = 20;
				break;

				case 2:
				note = MA_1000*4;
				dur = 20;
				break;

				case 3:
				note = MA_F7;
				dur = 20;
				break;

				case 4:
				note = MA_G7;
				dur = 20;
				break;

				case 5:
				note = MA_A7;
				dur = 20;
				break;

				case 6:
				note = MA_B7;
				dur = 20;
				break;

				case 7:
				note = MA_C8;
				dur = 20;
				break;

				case 8:
				note = MA_D8;
				dur = 20;
				break;

				case 9:
				note = MA_E8;
				dur = 20;
				break;

				case 10:
				note = MA_E8;
				dur = 500;
				break;

				case 11:
				note = 0;
				dur = 250;
				break;

				case 12:
				note = MA_A6;
				dur = 250;
				break;

				case 13:
				note = MA_B6;
				dur = 500;
				break;

				default:
				ma_step = 0;
				note = MA_C5;
				dur = 500;
				break;
			}
			if(++ma_step > 11) ma_step = 0;
			pause = true;
		}
		ma_tone(note, dur);
	}
	#else
	uint16_t	dur;
	static bool	pause = false;
	if(!toneON)
	{
		if(pause)
		{
			note = 0;
			dur = 200;
			pause = false;
		}
		else
		{
			switch(ma_step)
			{
				case 0:
				note = MA_B4;
				dur = 250;
				break;

				case 1:
				note = MA_B4;
				dur = 250;
				break;

				case 2:
				note = MA_B4;
				dur = 500;
				break;

				case 3:
				note = MA_B4;
				dur = 250;
				break;

				case 4:
				note = MA_B4;
				dur = 250;
				break;

				case 5:
				note = MA_B4;
				dur = 500;
				break;

				case 6:
				note = MA_B4;
				dur = 250;
				break;

				case 7:
				note = MA_D5;
				dur = 250;
				break;

				case 8:
				note = MA_G4;
				dur = 250;
				break;

				case 9:
				note = MA_A4;
				dur = 250;
				break;

				case 10:
				note = MA_B4;
				dur = 500;
				break;

				case 11:
				note = 0;
				dur = 500;
				break;

				case 12:
				note = MA_A6;
				dur = 500;
				break;

				case 13:
				note = MA_B6;
				dur = 500;
				break;

				default:
				ma_step = 0;
				note = MA_C5;
				dur = 500;
				break;
			}
			if(++ma_step > 11) ma_step = 0;
			pause = true;
		}
		ma_tone(note, dur);
	}

	#endif
	#endif
	
}

// trigger sin wave tone.
void ma_tone( uint16_t freq, uint16_t duration )
{
	accumStep = freq;
	ma_duration = duration;
	ma_loopCount = 78;						// initialize loop count.
	toneON = true;
	TIMSK1 |= (1<<TOIE1);					// enable interrupt on overflow.

	SPEAKER_DDR |= (1<<SPEAKER_POUT);		// set as output. Turn ON.

}

/*
* Output Tone for Duration. Clear toneON flag when done.
*/
ISR(TIMER1_OVF_vect)
{
	#if 1
	OCR1A = pwmValue;

	accumulator += accumStep;
	pwmValue = (uint16_t)(ma_getSinVal(accumulator>>8) + 128);		// use upper 8 bits for table index.

	if (--ma_loopCount == 0)
	{
		if (--ma_duration == 0)
		{
			toneON = false;
			SPEAKER_DDR &= ~(1<<SPEAKER_POUT);		// set as input. Turn OFF.
			TIMSK1 = 0;								// disable interrupt on overflow.
			} else {
			ma_loopCount = 78;
		}
	}
	#else
	//	OCR1A = (++pwmValue) & 0x00FF;
	OCR1A = 0x0040;
	#endif
}


/*
* r25:r24 = index (H,L)
* returns (char)r25:24 = ma_sinTable[index]
* Z = r31:30
*
* 256 entry sin() table
*/
char ma_getSinVal( uint8_t index )
{
	// get sin(index)
	//	return( pgm_read_byte(&ma_sinTable[index]) );
	return( pgm_read_byte(&ma_sinTable112[index]) );
}

const char ma_sinTable[] PROGMEM = {
	0,   3,   6,   9,  12,  15,  18,  21,  24,  27,  30,  33,  36,  39,  42,  45,
	48,  51,  54,  57,  59,  62,  65,  67,  70,  73,  75,  78,  80,  82,  85,  87,
	89,  91,  94,  96,  98, 100, 102, 103, 105, 107, 108, 110, 112, 113, 114, 116,
	117, 118, 119, 120, 121, 122, 123, 123, 124, 125, 125, 126, 126, 126, 126, 126,
	127, 126, 126, 126, 126, 126, 125, 125, 124, 123, 123, 122, 121, 120, 119, 118,
	117, 116, 114, 113, 112, 110, 108, 107, 105, 103, 102, 100,  98,  96,  94,  91,
	89,  87,  85,  82,  80,  78,  75,  73,  70,  67,  65,  62,  59,  57,  54,  51,
	48,  45,  42,  39,  36,  33,  30,  27,  24,  21,  18,  15,  12,   9,   6,   3,
	0,  -4,  -7, -10, -13, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46,
	-49, -52, -55, -58, -60, -63, -66, -68, -71, -74, -76, -79, -81, -83, -86, -88,
	-90, -92, -95, -97, -99,-101,-103,-104,-106,-108,-109,-111,-113,-114,-115,-117,
	-118,-119,-120,-121,-122,-123,-124,-124,-125,-126,-126,-127,-127,-127,-127,-127,
	-127,-127,-127,-127,-127,-127,-126,-126,-125,-124,-124,-123,-122,-121,-120,-119,
	-118,-117,-115,-114,-113,-111,-109,-108,-106,-104,-103,-101, -99, -97, -95, -92,
	-90, -88, -86, -83, -81, -79, -76, -74, -71, -68, -66, -63, -60, -58, -55, -52,
	-49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -13, -10,  -7,  -4
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
