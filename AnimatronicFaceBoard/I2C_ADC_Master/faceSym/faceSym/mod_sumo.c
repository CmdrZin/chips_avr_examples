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
 * mod_sumo.c
 *
 * Created: 7/27/2016 10:54:23 AM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "mod_sumo.h"
#include "sysTimer.h"
#include "adc_support.h"
#include "pwm_support.h"

void mss_runMode();
bool mss_atBarrier();


#define MSS_DELAY			10		// N * 10ms
#define MSS_RUN_DELAY		1		// update motors every N * 10ms
#define MSS_BARRIER_DELAY	75

#define	MSS_BTV				0x30

#define MSS_RANGE_VAL		0x30
#define MAX_RNG_DIFF		0x04
#define MSS_ATK_ADJ			0x30

MSS_STATES	mss_state;

uint8_t	mss_delay_count;

uint8_t	mss_count;

uint8_t	mss_barrierThreshold;

void mod_sumo_init()
{
	mss_state = MSS_IDLE;
	mss_delay_count = MSS_DELAY;
	mss_count = 0;
	mss_barrierThreshold = MSS_BTV;
}

void mod_sumo_service()
{
	if( GPIOR0 & (1<<MSS_10MS_TIC) )
	{
		GPIOR0 &= ~(1<<MSS_10MS_TIC);

		// First: Check for BARRIER.
		if( mss_atBarrier() && ((mss_state == MSS_RUN) || (mss_state == MSS_ATK)) )
		{
			mss_state = MSS_BARRIER;
			mss_delay_count = MSS_BARRIER_DELAY;
		}

		switch( mss_state )
		{
			case MSS_IDLE:
				// Check for BUTTON_GRN press. If pressed, go to MSS_RUN
				if(++mss_count > 250)
				{
					mss_count = 0;
					mss_state = MSS_RUN;
				}
				break;

			case MSS_RUN:
				if( --mss_delay_count == 0 )
				{
					mss_delay_count = MSS_RUN_DELAY;
					mss_runMode();
				}
				break;
			
			case MSS_ATK:
				// USE THIS FOR NOW.
				if( --mss_delay_count == 0 )
				{
					mss_delay_count = MSS_RUN_DELAY;
					mss_runMode();
				}
				break;
			
			case MSS_BARRIER:
				if( mss_delay_count == MSS_BARRIER_DELAY)
				{
					if( adc_support_readLine( LINE_R ) > mss_barrierThreshold ) {
						pwm_setMotor( PWM_L, PWM_L_MED_FWD );
						pwm_setMotor( PWM_R, PWM_R_MED_REV );						
					} else {
						pwm_setMotor( PWM_L, PWM_L_MED_REV );
						pwm_setMotor( PWM_R, PWM_R_MED_FWD );
					}
				}
				if( --mss_delay_count == 0)
				{
					pwm_setMotor( PWM_L, PWM_L_MED_FWD );
					pwm_setMotor( PWM_R, PWM_R_MED_FWD );
					mss_delay_count = MSS_RUN_DELAY;
					mss_state = MSS_RUN;
				}
				break;
			
			default:
				mss_state = MSS_IDLE;
				break;
		}
	}
}

/*
 * Run (Search) Mode
 *
 */
void mss_runMode()
{
	uint8_t ir_range[6];
	uint8_t	ir_group;
	uint8_t rangeDiff;
	uint8_t flag;
	
	ir_group = 0;

	// Copy all IR range readings.
	for( uint8_t i=0; i<6; i++ )
	{
		ir_range[i] = adc_support_readChan(i);
	}
	
	if( ir_range[IR_PT_FR] > ir_range[IR_PT_FL] ) {
		rangeDiff = ir_range[IR_PT_FR] - ir_range[IR_PT_FL];
		flag = IR_PT_FR;
	} else {
		rangeDiff = ir_range[IR_PT_FL] - ir_range[IR_PT_FR];
		flag = IR_PT_FL;
	}
	
	if( rangeDiff < MAX_RNG_DIFF ) {
		ir_group = 0;
	} else {
		ir_group = 2;
	}

	// Set motors
	switch( ir_group )
	{
		case 0:
			pwm_setMotor( PWM_L, PWM_L_MED_FWD );
			pwm_setMotor( PWM_R, PWM_R_MED_FWD );
			mss_state = MSS_RUN;
			break;
		
		case 2:
			if( flag == IR_PT_FL ) {
				// go left more
				pwm_setMotor( PWM_L, PWM_L_MED_FWD );
				pwm_setMotor( PWM_R, PWM_R_FAST_FWD );
			} else {
				// go right more
				pwm_setMotor( PWM_L, PWM_L_FAST_FWD );
				pwm_setMotor( PWM_R, PWM_R_MED_FWD );		// opposite of FWD.
			}
			mss_state = MSS_ATK;
			break;
		
		default:
			pwm_setMotor( PWM_L, PWM_L_STOP );
			pwm_setMotor( PWM_R, PWM_R_STOP );
			break;
	}
}

/*
 * Check if at Barrier
 */
bool mss_atBarrier()
{
	bool result = false;
	
	result = (adc_support_readLine( LINE_L ) > mss_barrierThreshold) ? true : result;
	result = (adc_support_readLine( LINE_C ) > mss_barrierThreshold) ? true : result;
	result = (adc_support_readLine( LINE_R ) > mss_barrierThreshold) ? true : result;

	return( result );
}

MSS_STATES mod_sumo_getState()
{
	return( mss_state );
}
