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
 * SumoBotC.c
 *
 * Created: 7/19/2016 12:49:30 PM
 *  Author: Chip
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
#include "twi_I2CMaster.h"
#include "mod_stdio.h"
#include "adc_support.h"
#include "pwm_support.h"
#include "mod_sumo.h"


int main(void)
{
	int count = 0;
	char* sumo_state;
	char outBuff[8];
	
	uint8_t report_state = 0;
	
//	uint16_t mVal;
	
	st_init_tmr0();
	tim_init();
	adc_support_init();
	pwm_support_init();
	mod_sumo_init();
	
	sei();				// Enable interrupts
	
    while(1)
    {
		adc_support_service();
		mod_sumo_service();

	
		if( GPIOR0 & (1<<DEV_1MS_TIC) )
		{
			GPIOR0 &= ~(1<<DEV_1MS_TIC);

			if( tim_isBusy() == false ) {

			switch( count )
			{
				case 0:
#if 0
					mVal = pwm_getMotor(PWM_L);
#if 0
					if(mVal < PWM_L_FAST_FWD) {
						pwm_setMotor(PWM_L, mVal + 4);
						} else {
						pwm_setMotor(PWM_L, PWM_L_STOP);
					}
#endif
#if 0
					if(mVal == PWM_L_STOP) {
						pwm_setMotor(PWM_L, PWM_L_SLOW_FWD);
					} else if(mVal == PWM_L_SLOW_FWD) {
						pwm_setMotor(PWM_L, PWM_L_MED_FWD);
					} else if(mVal == PWM_L_MED_FWD) {
						pwm_setMotor(PWM_L, PWM_L_FAST_FWD);
					} else {
						pwm_setMotor(PWM_L, PWM_L_STOP);
					}
#endif
#if 0
					if(mVal == PWM_L_STOP) {
						pwm_setMotor(PWM_L, PWM_L_SLOW_REV);
					} else if(mVal == PWM_L_SLOW_REV) {
						pwm_setMotor(PWM_L, PWM_L_MED_REV);
					} else if(mVal == PWM_L_MED_REV) {
						pwm_setMotor(PWM_L, PWM_L_FAST_REV);
					} else {
						pwm_setMotor(PWM_L, PWM_L_STOP);
					}
#endif
					mod_stdio_bin2hex(&outBuff[0], (uint8_t)(mVal>>8));
					outBuff[0] = 'L';
					mod_stdio_bin2hex(&outBuff[2], (uint8_t)(mVal & 0x00FF));
					mVal = pwm_getMotor(PWM_R);
#if 0
					if(mVal > PWM_R_FAST_FWD) {
						pwm_setMotor(PWM_R, mVal - 4);
						} else {
						pwm_setMotor(PWM_R, PWM_R_STOP);
					}
#endif
#if 0
					if(mVal == PWM_R_STOP) {
						pwm_setMotor(PWM_R, PWM_R_SLOW_FWD);
					} else if(mVal == PWM_R_SLOW_FWD) {
						pwm_setMotor(PWM_R, PWM_R_MED_FWD);
					} else if(mVal == PWM_R_MED_FWD) {
						pwm_setMotor(PWM_R, PWM_R_FAST_FWD);
					} else {
						pwm_setMotor(PWM_R, PWM_R_STOP);
					}
#endif
#if 0
					if(mVal == PWM_R_STOP) {
						pwm_setMotor(PWM_R, PWM_R_SLOW_REV);
					} else if(mVal == PWM_R_SLOW_REV) {
						pwm_setMotor(PWM_R, PWM_R_MED_REV);
					} else if(mVal == PWM_R_MED_REV) {
						pwm_setMotor(PWM_R, PWM_R_FAST_REV);
					} else {
						pwm_setMotor(PWM_R, PWM_R_STOP);
					}
#endif
					mod_stdio_bin2hex(&outBuff[4], (uint8_t)(mVal>>8));
					outBuff[4] = 'R';
					mod_stdio_bin2hex(&outBuff[6], (uint8_t)(mVal & 0x00FF));
					mod_stdio_print(1, outBuff, 8);
					break;
#else
					switch( mod_sumo_getState() )
					{
						case MSS_IDLE:
							sumo_state = "SS: IDLE";
							break;
							
						case MSS_RUN:
							sumo_state = "SS: RUN ";
							break;
							
						case MSS_ATK:
							sumo_state = "SS: ATK ";
							break;
							
						case MSS_BARRIER:
							sumo_state = "SS: BARR";
							break;
							
						default:
							sumo_state = "SS: OOPS";
							break;
					}
					mod_stdio_print(1, sumo_state, 8);
					break;
#endif
				case 1:
					switch( report_state )
					{
						case 0:
							outBuff[0] = ' ';
							outBuff[1] = 'R';
							mod_stdio_bin2hex( &outBuff[2], adc_support_readChan(IR_PT_FR));
							outBuff[4] = ' ';
							outBuff[5] = 'L';
							mod_stdio_bin2hex( &outBuff[6], adc_support_readChan(IR_PT_FL));
							mod_stdio_print(2, outBuff, 8);
							break;

						case 1:
							outBuff[0] = ' ';
							outBuff[1] = 'F';
							outBuff[2] = 'L';
							outBuff[3] = ':';
							mod_stdio_bin2hex( &outBuff[4], adc_support_readChan(IR_PT_FL));
							outBuff[6] = ' ';
							outBuff[7] = ' ';
							mod_stdio_print(2, outBuff, 8);
							break;
							
						default:
							report_state = 0;
					}
//					if( ++report_state > 1 ) { report_state = 0; }
					break;

				case 40:
					mod_stdio_led( MOD_IO_GREEN_LED, true );
					break;
				
				case 60:
					mod_stdio_led( MOD_IO_RED_LED, true );
					break;
			
				case 80:
					mod_stdio_led( MOD_IO_YELLOW_LED, true );
					break;
			
				case 100:
					mod_stdio_led( MOD_IO_GREEN_LED, false );
					break;
				
				case 120:
					mod_stdio_led( MOD_IO_RED_LED, false );
					break;
				
				case 140:
					mod_stdio_led( MOD_IO_YELLOW_LED, false );
					break;
			}
			if( ++count > 200 ) { count = 0; }
			}
		}
    }
}
