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
 * main.c
 *
 * Created: 10/14/2018	0.01	ndp
 *  Author: Chip
 *
 * Demo code for RC Decoder Slave project.
 * Target: ATmega88A (be sure to set Project > Properties > Device to this AVR chip)
 * NOTE: Uncheck the CKDIV8 fuse to use an 8 MHz CPU clock.
 *
 * This code demonstrates a simple RC Servo signal decoder.
 * It will also allow the Master to set the PWM for the Steering and Throttle.
 *
 * Commands:
 * SYNC 0x00 STATE		Controls the LED. STATE = 0:OFF, 1:ON
 * SYNC 0x01 TT			Set Throttle to TT (0x00 Full Reverse..0x80 Stop..0xFF Full Forward)
 * SYNC 0x02 SS			Set Steering to SS (0x00 Full Left..0x80 Center..0xFF Full Right)
 * SYNC 0x10			Returns temperature data (ADCH ADCL '\n') once per second.
 * SYNC 0x11 			Returns Throttle and Steering data (TT ST '\n') every 20ms.
 *
 * txBuf[0] = 0x00 		always
 * txBuf[1] = ADCH		Left shifted..upper 8 bits
 * txBuf[2] = ADCL		Lower 2 bits
 * txBuf[3] = TROTTLE	One byte
 * txBuf[4] = STEERING	One byte
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
#include "serial.h"
#include "mod_comms.h"
#include "mod_led.h"
#include "mod_adc.h"
#include "mod_rcDecode.h"


void test01(int c);

typedef enum {IDLE, READ_DATA} RX_STATES;

int main(void)
{
	long timeWait = 0;

	st_init_tmr0();
	mod_led_init();
	initUsart();
	mod_adc_init();
	mod_rcDecode_init();

	// TEST DATA LOAD
	mod_comm_setReg( 1, 0x11 );			// ADCH
	mod_comm_setReg( 2, 0x22 );			// ADCL
	mod_comm_setReg( 3, 0x33 );			// THROTTLE
	mod_comm_setReg( 4, 0x44 );			// STEERING

	sei();							// Enable interrupts.
	
	test01(6);

	while(1)
	{
		if( timeWait < st_millis() ) {
			mod_adc_getAndTrigger();				// update temperature registers.
			timeWait = st_millis() + 1000;			// set to 1 sec wait.
		}

		mod_comms_service();
	}
}

void test01(int cnt)
{
	long nextTime = st_millis() + 500;
	uint8_t tog = 0;
	int count = 0;
	
	while(count < cnt) {
		if(nextTime < st_millis()) {
			nextTime = nextTime + 500;
			++count;				// only inc if timeout occurs.
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
