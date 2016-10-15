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
 * mod_system.c
 *
 * Created: 7/27/2016 10:54:23 AM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "mod_system.h"
#include "sysTimer.h"
#include "mod_led.h"
#include "adc_support.h"
#include "pwm_support.h"
#include "twi_I2CMaster.h"
#include "mod_stdio.h"
#include "mod_pca9685.h"

void mss_runMode();
void mss_manualMode();

#define MSS_DELAY			10		// N * 10ms
#define MSS_RUN_DELAY		50		// N * 10ms
#define MSS_MANUAL_DELAY	2		// N * 10ms


MSS_STATES	mss_state;

uint8_t	mss_delay_count;

uint8_t	mss_count;

void mod_system_init()
{
	mss_state = MSS_IDLE;
	mss_delay_count = MSS_DELAY;
	mss_count = 0;
}

/*
 * System Service
 * Servos are on a 20ms period, so it doesn't doing good to try to update them faster than that.
 *
 */
void mod_system_service()
{
	if( GPIOR0 & (1<<MSS_10MS_TIC) )
	{
		GPIOR0 &= ~(1<<MSS_10MS_TIC);

		switch( mss_state )
		{
			case MSS_IDLE:
				// Check for BUTTON_GRN press. If pressed, go to MSS_RUN
				if(++mss_count > 250)
				{
					// Send out banner.
					while( tim_isBusy());
					mod_stdio_print(1, "The Face", 8);
					while( tim_isBusy());
					mod_stdio_print(2, " Is Up! ", 8);

					mss_count = 0;
//					mss_state = MSS_RUN;
//					mss_delay_count = MSS_RUN_DELAY;
					mss_state = MSS_MANUAL;
					mss_delay_count = MSS_MANUAL_DELAY;
				}
				break;

			case MSS_RUN:
				if( --mss_delay_count == 0 )
				{
					mss_delay_count = MSS_RUN_DELAY;
					mss_runMode();
				}
				break;
			
			// MANUAL Mode has the servo track its associated control pot.
			case MSS_MANUAL:
				if( --mss_delay_count == 0 )
				{
					mss_delay_count = MSS_MANUAL_DELAY;
					mss_manualMode();
				}
				break;

			default:
				mss_state = MSS_IDLE;
				break;
		}
	}
}

/*
 * Run Mode
 */
void mss_runMode()
{
	static uint8_t slow_cnt = 0;

			++slow_cnt;
			if( slow_cnt == 1) {
				mp_setPos(2, 0x0133+60);
				mod_led_on();

			}
			if( slow_cnt == 2) {
				mp_setPos(2, 0x0133-60);
				mod_led_off();
				slow_cnt = 0;
			}
}

/*
 * Manual Mode
 */
void mss_manualMode()
{
	int adc_data;

	for( uint8_t i=0; i<8; i++)
	{
		// Get ADC channels last reading.
		adc_data = (int)adc_support_readChan(i);			// 0 -> 255
		adc_data -= 128;									// center

		mp_setPos(i, MP_CENTER - adc_data);

		if(i == 0) {
			mod_stdio_print2Hex("00", 0, (uint8_t)adc_data);
		}
	}
}

MSS_STATES mod_system_getState()
{
	return( mss_state );
}
