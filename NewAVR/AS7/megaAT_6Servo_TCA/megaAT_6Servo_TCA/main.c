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
 * megaAT_6Servo_TCA
 *
 * Created: 11/19/2020 9:16:18 PM
 * Author : Chip
 *
 * Demo code for:
 *   TCAservo:		6 PWM Servo control
 *
 * NOTE: Assumes CPU_CLK = 16 MHz with OSC20M fuse set for 16 MHz
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#include "sysTime.h"
#include "mod_led.h"
#include "TCA6servo.h"

#define LED_DELAY	500UL		// N * 1ms
#define PWM_DELAY	20UL		// N * 1ms

int main(void)
{
	static uint32_t ledTime = 0UL;
	static uint32_t pwmTime = 0UL;
	static uint8_t newPosition = 0;		// keep between 0 and 125
	static bool countUp = true;
	
	mod_led_init();
	init_time();
	TCA0servo_init();
	
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
#if 1
			// Quick test
			if( ++newPosition > 125 ) {
				newPosition = 0;
			}
			if( (newPosition & 0x03) == 0x03 ) {
				TCAservo_setPWM0(0);
				TCAservo_setPWM1(0);
				TCAservo_setPWM2(0);
				TCAservo_setPWM3(0);
				TCAservo_setPWM4(0);
				TCAservo_setPWM5(0);
			} else if( (newPosition & 0x02) == 0x02 ) {
				TCAservo_setPWM0(63);
				TCAservo_setPWM1(63);
				TCAservo_setPWM2(63);
				TCAservo_setPWM3(63);
				TCAservo_setPWM4(63);
				TCAservo_setPWM5(63);
			} else {
				TCAservo_setPWM0(125);
				TCAservo_setPWM1(125);
				TCAservo_setPWM2(125);
				TCAservo_setPWM3(125);
				TCAservo_setPWM4(125);
				TCAservo_setPWM5(125);
			}
#else			
			// Ramp up and down test
			if(countUp) {
				if( newPosition < 125 ) {
					++newPosition;
				} else {
					--newPosition;
					countUp = false;
				}
			} else {
				if( newPosition > 0 ) {
					--newPosition;
				} else {
					++newPosition;
					countUp = true;
				}
			}
			TCAservo_setPWM0(newPosition);
			TCAservo_setPWM1(newPosition);
			TCAservo_setPWM2(newPosition);
			TCAservo_setPWM3(newPosition);
			TCAservo_setPWM4(newPosition);
			TCAservo_setPWM5(newPosition);
#endif
		}
    }
}
