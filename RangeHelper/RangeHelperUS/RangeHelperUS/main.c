/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * RangeHelperUS.c
 *
 * Created: 12/1/2018 12:34:35 PM
 * Author : Chip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
#include "mod_led.h"
#include "usRange.h"
#include "vMotor.h"


void test();
void mtest();


int main(void)
{
	st_init_tmr0();
	mod_led_init();
	usr_init();
	vm_init();
	
	sei();
	
	mod_led_toggle(3);

    while (1) 
    {
		if( usr_service() == 1 ) {
			// Update vmotor every 50ms or so
			st_setPWM(255 - usr_getRange());			// Add vMotor code to map range to speed.
//			mtest();
			if(usr_getRange() < 128) {
				mod_led_on();
			} else {
				mod_led_off();
			}
		}
    }
}

void mtest()
{
	static uint32_t	m_lastTime = 0;
	static uint8_t	count = 0;

    if( m_lastTime < st_millis() ) {
		m_lastTime = m_lastTime + 1;
		st_setPWM(count++);
	}
}

void test()
{
	static uint8_t		ledFlag = 0;
	static uint32_t		m_lastTime = 0;

    while (1)
    {
	    if( m_lastTime < st_millis() ) {
		    m_lastTime = m_lastTime + 1000;
		    if( ledFlag == 0) {
			    mod_led_off();
			    ledFlag = 1;
		    } else {
			    mod_led_on();
			    ledFlag = 0;
		    }
	    }
	    
    }
}
