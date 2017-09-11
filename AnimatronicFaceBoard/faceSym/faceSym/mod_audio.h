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
 */ 


#ifndef MOD_AUDIO_H_
#define MOD_AUDIO_H_

// Notes
#define MA_C3		105				// C3 130.81 Hz
#define MA_D3		117				// D3 146.83 Hz
#define MA_E3		132				// E3 164.81 Hz
#define MA_F3		140				// F3 174.61 Hz
#define MA_G3		157				// G3 196.00 Hz
#define MA_A3		176				// A3 220.00 Hz
#define MA_B3		198				// B3 246.94 Hz
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
#define MA_A5		(MA_A3*4)		// A5 880.00 Hz..704
#define MA_B5		(MA_B3*4)		// 792
#define MA_C6		(MA_C3*8)		// 840
#define MA_D6		(MA_D3*8)
#define MA_E6		(MA_E3*8)
#define MA_F6		(MA_F3*8)
#define MA_G6		(MA_G3*8)
#define MA_A6		(MA_A3*8)		// A5 1760.00 Hz
#define MA_B6		(MA_B3*8)
#define MA_C7		(MA_C3*16)
#define MA_D7		(MA_D3*16)
#define MA_E7		(MA_E3*16)
#define MA_F7		(MA_F3*16)
#define MA_G7		(MA_G3*16)
#define MA_A7		(MA_A3*16)		// A5 3520.00 Hz
#define MA_B7		(MA_B3*16)
#define MA_C8		(MA_C3*32)
#define MA_D8		(MA_D3*32)
#define MA_E8		(MA_E3*32)
#define MA_F8		(MA_F3*32)
#define MA_G8		(MA_G3*32)
#define MA_A8		(MA_A3*32)		// A5 7040.00 Hz
#define MA_B8		(MA_B3*32)

#define MA_1000		800				// 1kHz
#define MA_10000	(32*256)		// 10kHz

void ma_init();

void ma_service();

void ma_tone( uint16_t freq, uint16_t duration );


char ma_getSinVal( uint8_t index );



#endif /* MOD_AUDIO_H_ */