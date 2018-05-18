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

#include "sysTimer.h"
#include "ws2812_8_util.h"
#include "mod_led.h"

void test01(void);

#define STEP_TIME	5		// N * ms

#define NUM_OF_PIXS	3
#define BYTE_CHAIN	(NUM_OF_PIXS * 3)
char buf[12];

int main(void)
{
	long nextTime = st_millis() + STEP_TIME;
	uint8_t state = 1;		// pick pattern

	int flag = 0;
	buf[0] = 0x00;		// Green
	buf[1] = 0x00;		// Red
	buf[2] = 0x00;		// Blue
	buf[3] = 0x00;		// Green
	buf[4] = 0x00;		// Red
	buf[5] = 0x00;		// Blue
	buf[6] = 0x00;		// Green
	buf[7] = 0x00;		// Red
	buf[8] = 0x00;		// Blue

	st_init_tmr0();
	mod_led_init();

	sei();						// global enable interrupts

	while(1) {
		if(nextTime < st_millis()) {
			nextTime = nextTime + STEP_TIME;

			ws2812_8_update((void*)buf, BYTE_CHAIN);

			switch( state ) {
				case 0:
					// Simple test
					if(flag == 0) {
						if(buf[0] != 255) {
							++buf[0];
							++buf[4];
						} else {
							if(buf[2] != 255) {
								++buf[2];
								++buf[3];
							} else {
								if(buf[1] != 255) {
									++buf[1];
									++buf[5];
								} else {
									flag = 1;
								}
							}
						}
					} else {
						if(buf[0] != 0) {
							--buf[0];
							--buf[4];
						} else {
							if(buf[2] != 0) {
								--buf[2];
								--buf[3];
							} else {
								if(buf[1] != 0) {
									--buf[1];
									--buf[5];
								} else {
									flag = 0;
								}
							}
						}
					}
					break;

				case 1:
					// Simple test
					if(flag == 0) {
						if(buf[0] != 255) {
							++buf[0];
							++buf[4];
							++buf[8];
						} else {
							flag = 1;
						}
					} else {
						if(buf[0] != 0) {
							--buf[0];
							--buf[4];
							--buf[8];
						} else {
							flag = 0;
						}
					}
					break;
					
				default:
					state = 0;
					break;
			}
		}
	}

	return(0);
}

void test01(void)
{
	long nextTime = st_millis() + STEP_TIME;
	uint8_t tog = 0;
	
	while(1) {
		if(nextTime < st_millis()) {
			nextTime = nextTime + STEP_TIME;
			if(tog == 0) {
				tog = 1;
				mod_led_on();
				} else {
				tog = 0;
				mod_led_off();
			}
		}
	}
}
