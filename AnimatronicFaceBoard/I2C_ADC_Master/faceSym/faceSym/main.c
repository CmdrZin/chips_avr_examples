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
 * faceSym.c
 *
 * Created: 9/11/2016	v0.01	ndp
 * Author : Chip
 *
 * Target: Atmega164PA, 20 MHz
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
#include "mod_led.h"
#include "twi_I2CMaster.h"
#include "mod_stdio.h"
#include "adc_support.h"
#include "pwm_support.h"
//#include "mod_sumo.h"
#include "mod_serial.h"


int main(void)
{
	int count = 0;
	
	st_init_tmr0();
	mod_led_init();
	tim_init();
	adc_support_init();
	pwm_support_init();
//	mod_sumo_init();
	mod_serial_init();
	
	sei();				// Enable interrupts
	
	while(1)
	{
		adc_support_service();
//		mod_sumo_service();

		GPIOR0 &= ~(1<<MSS_10MS_TIC);
		while( !(GPIOR0 & (1<<MSS_10MS_TIC)) );
		while( tim_isBusy());
		mod_stdio_print(1, "The Face", 8);
		while( tim_isBusy());
		mod_stdio_print(2, " Is Up! ", 8);

		
		if( GPIOR0 & (1<<DEV_1MS_TIC) )
		{
			GPIOR0 &= ~(1<<DEV_1MS_TIC);

			if(count == 0) {
				mod_led_on();
			}
			if(count == 100) {
				mod_led_off();
			}


//			if( tim_isBusy() == false ) {

				if( ++count > 200 ) { count = 0; }
//			}
		}
	}
}
