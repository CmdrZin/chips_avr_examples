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
 */

/*
 * mod_led_status.h
 *
 * Created: 8/13/2015 4:34:10 PM
 *  Author: Chip
 */ 


#ifndef MOD_LED_STATUS_H_
#define MOD_LED_STATUS_H_

#include <stdbool.h>

#define MOD_LED_STATUS_ID	0x01

#define MLS_TURN_ON_LED			0x01
#define MLS_TURN_OFF_LED		0x02


void mod_led_status_init();
void mod_led_status_service();

void mod_led_status_on();
void mod_led_status_off();

void mls_setRangeStatus( uint8_t led, bool state );
void mls_turn_led_on( uint8_t val );
void mls_turn_led_off( uint8_t val );


#endif /* MOD_LED_STATUS_H_ */