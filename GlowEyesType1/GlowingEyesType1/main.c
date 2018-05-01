/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016-2018 Nels D. "Chip" Pearson (aka CmdrZin)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
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
 * GlowingEyesType1 (main.c)
 *
 * Created: 12/2/2016 2:07:50 PM
 * Author : Chip
 * Revised: 04/30/2018		0.10	ndp	mod for use by Glowing Eyes Type 1
 *
 * Two LEDs are connected to the two PWM outputs for Timer0. OC0A and OC0B.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "mod_pwm.h"
#include "flash_table.h"

#define STEP_TIME	10			// N * ms

int main(void)
{
	long nextTime = st_millis() + STEP_TIME;
	uint8_t rampValue = 0;
	uint8_t rampAdj = 1;
	
	mod_pwm_init();				// initialize PWM timer

	sei();						// global enable interrupts

	while(1) {
		if(nextTime < st_millis()) {
			nextTime = nextTime + STEP_TIME;
			mod_pwm1_set(rampValue);
			mod_pwm2_set(rampValue);
			rampValue += rampAdj;
			if(rampValue == 0xFF) {
				rampAdj = -1;
			}
			if(rampValue == 0) {
				mod_pwn_disable();
				MCUCR = (1<<SE)|(1<<SM1)|(0<<SM0);			// Use RESET to WakeUp.
				asm("sleep");
				rampAdj = 1;
			}
		}
	}
}
