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
* mod_audio.h
*
* Created: 9/24/2016 11:07:52 PM
*  Author: Chip
* revision:	2/08/2017		0.02	ndp	convert for 1MHz clock
*/


#ifndef MOD_AUDIO_H_
#define MOD_AUDIO_H_

/*
* MA_ISR_SQUARE		Square wave tone for duration.
* MA_ISR_SIN			Sin wave tone for duration.
* MA_ISR_TRIANGLE		Triangle wave tone for duration.
* MA_ISR_SAW			Sawtooth wave tone for duration.
*/
typedef enum ma_isr {MA_ISR_SQUARE, MA_ISR_SIN, MA_ISR_TRIANGLE, MA_ISR_SAW} MA_ISR_SERVICE;

typedef enum ma_serviceType {MA_SERVICE_NONE, MA_SERVICE_BEEP, MA_SERVICE_SCORE1, MA_SERVICE_SCORE2, 
	MA_SERVICE_SCORE3, MA_SERVICE_SCORE4, MA_SERVICE_SCORE5, MA_SERVICE_SCORE6, MA_SERVICE_SCORE7} MA_SERVICE_TYPE;


// Notes..1MHz clock using DDS and 256 point sin wave table.
#define MA_C3		(105*20)		// C3 130.81 Hz
#define MA_D3		(117*20)		// D3 146.83 Hz
#define MA_E3		(132*20)		// E3 164.81 Hz
#define MA_F3		(140*20)		// F3 174.61 Hz
#define MA_G3		(157*20)		// G3 196.00 Hz
#define MA_A3		(176*20)		// A3 220.00 Hz
#define MA_B3		(198*20)		// B3 246.94 Hz
#define MA_C4		(MA_C3*2)
#define MA_D4		(MA_D3*2)
#define MA_E4		(MA_E3*2)
#define MA_F4		(MA_F3*2)
#define MA_G4		(MA_G3*2)
#define MA_A4		(MA_A3*2)		// A4 440.00 Hz
#define MA_B4		(MA_B3*2)
#define MA_C5		(MA_C3*4)
#define MA_D5		(MA_D3*4)
#define MA_E5		(MA_E3*4)
#define MA_F5		(MA_F3*4)
#define MA_G5		(MA_G3*4)
#define MA_A5		(MA_A3*4)		// A5 880.00 Hz
#define MA_B5		(MA_B3*4)
#define MA_C6		(MA_C3*8)
#define MA_D6		(MA_D3*8)
#define MA_E6		(MA_E3*8)
#define MA_F6		(MA_F3*8)
#define MA_G6		(MA_G3*8)
#define MA_A6		(MA_A3*8)		// A5 1760.00 Hz
#define MA_B6		(MA_B3*8)
#define MA_1000		(64*256)		// 1kHz
// (128*256)-1 is max value (2kHz) for a Note

// Reconfigure Timer1 to get higher frequencies.
#define MA_BEEP_1k	511			// 1kHz
#define MA_BEEP_2k	255			// 2kHz
#define MA_BEEP_3k	166			// 3kHz
#define MA_BEEP_4k	128			// 4kHz
#define MA_BEEP_5k	102			// 5kHz


void ma_init();
void ma_clearAudio();
void ma_setBeepParam(uint16_t freq, uint8_t duration, uint8_t repetition);
void ma_setService(MA_SERVICE_TYPE val);
bool ma_isPlaying();
void ma_service();
void ma_tone( uint16_t freq, uint16_t duration );
char ma_getSinVal( uint8_t index );


#endif /* MOD_AUDIO_H_ */