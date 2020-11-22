/*
 * The MIT License
 *
 * Copyright 2020 CmdrZin.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * megaAT_6Glow_TCA
 *
 * Created: 11/19/2020 9:16:18 PM
 * Author : Chip
 *
 * Demo code for:
 *   TCA6pwm:		6 PWM signal generator
 *
 * NOTE: Assumes CPU_CLK = 16 MHz with OSC20M fuse set for 16 MHz
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#include "sysTime.h"
#include "mod_led.h"
#include "TCA6pwm.h"

#define LED_DELAY	500UL		// N * 1ms
#define PWM_DELAY	2UL			// N * 1ms

int main(void)
{
	static uint32_t ledTime = 0UL;
	static uint32_t pwmTime = 0UL;
	static uint8_t newDuty = 0;
	static bool countUp = true;
	
	mod_led_init();
	init_time();
	TCA_6pwm_init();
	
	sei();
	
    while (1) 
    {
#if 1
		if( st_millis() > ledTime ) {
			ledTime += LED_DELAY;
			mod_led_toggle(250);
		}
#endif
		/* Cycle counting up then down then up ... */
		if( st_millis() > pwmTime ) {
			pwmTime += PWM_DELAY;
			// Ramp up and down test
			if(countUp) {
				if( newDuty < 128 ) {
					++newDuty;
				} else {
					--newDuty;
					countUp = false;
				}
			} else {
				if( newDuty > 0 ) {
					--newDuty;
				} else {
					++newDuty;
					countUp = true;
				}
			}
			for( int i=0; i<6; i++) {
				TCA_setPWM(i, newDuty);
			}
		}
    }
}
