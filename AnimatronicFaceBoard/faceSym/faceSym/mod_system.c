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
#include <stdio.h>

#include "mod_system.h"
#include "sysTimer.h"
#include "mod_led.h"
#include "adc_support.h"
#include "twi_I2CMaster.h"
#include "mod_stdio.h"
#include "mod_pca9685.h"

void mss_runMode();
void mss_manualMode();

#define MOD_ADC_I2C	0x32

#define MSS_DELAY			10		// N * 1ms
#define MSS_RUN_DELAY		100		// N * 1ms
#define MSS_MANUAL_DELAY	25		// N * 1ms

typedef struct {
	uint32_t ktimeStamp;			// minimum 20ms steps. Maximum update rate.
	uint8_t	servo[16];
} SERVO_DATA;
SERVO_DATA	servoPos[32];		// 64 @ 20 bytes per entry -> 1280 bytes.
uint32_t seqStep;				// Duration between key frames.

MSS_STATES	mss_state;

uint8_t	mssDelay;

uint8_t	mss_count;

int currentAdc[16];			// latest ADC reading (-128 : 127)

void mod_system_init()
{
	mss_state = MSS_IDLE;
	mssDelay = MSS_DELAY;
	mss_count = 0;
	seqStep = 100;				// N * ms
}

void mod_systey_setKeyFrame(uint8_t servoNum, uint8_t pos)
{
	servoPos[0].servo[0] = servoNum;
	servoPos[0].ktimeStamp = 500;
}

/*
 * System Service
 * Servos are on a 20ms period, so it doesn't doing good to try to update them faster than that.
 *
 */
void mod_system_service()
{
	static uint32_t lastTime = 0;
	uint32_t currentTime;
	
	currentTime = st_millis();
	
	if( currentTime > (lastTime + mssDelay) )
	{
		lastTime = currentTime;

		switch( mss_state )
		{
			case MSS_IDLE:
				// Check for BUTTON_GRN press. If pressed, go to MSS_RUN
				if(mss_count == 1) {
					// Send out banner.
					mod_stdio_print("The Face Is Up! ", 16);
				}
				if(++mss_count > 250)
				{
					mss_count = 0;
//					mss_state = MSS_RUN;
//					mss_delay_count = MSS_RUN_DELAY;
					mss_state = MSS_MANUAL;
					mssDelay = MSS_MANUAL_DELAY;
					// Set up Manual text
					mod_stdio_print("Chan 1:         ", 16);
				}
				break;

			case MSS_RUN:
				mss_runMode();
				break;
			
			// MANUAL Mode has the servo track its associated control pot.
			case MSS_MANUAL:
				mss_manualMode();
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
	char buf[6];
	uint8_t nbytes;

	for( uint8_t i=0; i<8; i++)
	{
		// Get ADC channels last reading.
		adc_data = (int)adc_support_readChan(i);			// 0 -> 255
		adc_data -= 128;									// center
		
		currentAdc[i] = adc_data;

		mp_setPos(i, MP_CENTER - currentAdc[i]);			// using i+8 controls second set of 8 servos.

		if(i == 0) {
			nbytes = snprintf(buf, 7, " %+04d ", adc_data);
			mod_stdio_printOffset(buf, nbytes-1, 3);
		}
	}

#if 1
	// Read ADC Slave Ch 8-15
	buf[0] = 0x08;
	// Send packet.
	tim_write(MOD_ADC_I2C, (uint8_t*)buf, 1);
	
	// NOTE: Could use a time-out here of 5ms
	tim_read( MOD_ADC_I2C, 8 );
	for(int i=8; i<16; i++)
	{
		// Wait for data.
		while(!tim_hasData());
		// get data.
		adc_data = (int)tim_readData();
		adc_data -= 128;									// center

		currentAdc[i] =  adc_data;

		mp_setPos(i, MP_CENTER - currentAdc[i]);
	}
#endif
}

MSS_STATES mod_system_getState()
{
	return( mss_state );
}
