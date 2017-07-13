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
 * mod_4x2LedDisplay.c
 *
 * Created: 6/29/2017 5:09:00 PM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "sysTimer.h"
#include "mod_4x2LedDisplay.h"
#include "io_4x27SegLeds.h"


#define MLS_DIGIT_TIME	1		// N * ms

uint8_t	mls_delay;
MLD_STATE	mldState;

uint8_t mls_buffer[8];
uint8_t mls_bufDP[8];			// decimal point buffer..dp if not 0.

uint8_t	mls_nextDigit;
uint8_t	mls_nextPattern;

/*
 * Initialize Display Service
 */
void mod_LD_init()
{
	isl_init();				// set up hardware

	mldState = MLD_TEST;
	
	mls_nextDigit = 3;
	mls_nextPattern = 0;
	// Clear bufferS
	for(int i=0; i<8; i++) {
		mls_buffer[i] = 0x10;		// 0x10 space
		mls_bufDP[i] = 0;			// turn OFF dp.
	}
}

/*
 * Service display
  * Scan digits to multiplex display.
 */
void mod_LD_service()
{
	if( GPIOR0 & (1<<DEV_1MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_1MS_TIC);				// clear flag
		if( --mls_delay == 0 )
		{
			mls_delay = MLS_DIGIT_TIME;

			switch(mldState) {
				case MLD_TEST:
					// Display number for each digit position.
					isl_update(mls_nextDigit, mls_nextPattern);
					if(++mls_nextDigit >= 8) {
						mls_nextDigit = 0;
					}
					mls_nextPattern = isl_getPattern(mls_nextDigit);
					break;
					
				case MLD_NORMAL:
					// Display value from mls_line array.
					isl_update(mls_nextDigit, mls_nextPattern);
					if(++mls_nextDigit >= 8) {
						mls_nextDigit = 0;
					}
					mls_nextPattern = isl_getPattern(mls_buffer[mls_nextDigit]);
					break;
					
					
				default:
					mldState = MLD_TEST;
					break;
			}

			// Add Dp if needed.
			if(mls_bufDP[mls_nextDigit] != 0) {
				// turn on decimal point
				mls_nextPattern |= ISL_DECIMAL_POINT;
			}
		}
	}
}

void mod_LD_setMode(MLD_STATE state)
{
	mldState = state;
}

void mod_LD_setDigit(uint8_t digit, uint8_t val)
{
	mls_buffer[digit] = val;
}

/*
 * Set Decimal Point. DP will light if not 0.
 */ 
void mod_LD_setDP(uint8_t digit, uint8_t val)
{
	mls_bufDP[digit] = val;
}
