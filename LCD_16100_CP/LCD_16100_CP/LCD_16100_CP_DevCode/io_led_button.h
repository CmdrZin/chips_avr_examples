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
 * io_led_button.h
 *
 * Created: 7/24/2016 10:11:43 AM
 *  Author: Chip
 * revised: 9/15/2017	0.02	ndp		improve interface
 */ 


#ifndef IO_LED_BUTTON_H_
#define IO_LED_BUTTON_H_

#include <avr/io.h>
#include <stdbool.h>

#define MOD_IO_LED_BUTTON_ID	5

#define IO_GREEN_BUTTON		0x01
#define IO_RED_BUTTON		0x02
#define IO_YELLOW_BUTTON	0x04

typedef enum {IO_TEST, IO_SCAN} MOD_IO_STATE;


void mod_io_init();
void mod_io_service();

uint8_t mod_io_getButtons();

void mod_io_setGreenLed( uint8_t val );
void mod_io_setRedLed( uint8_t val );
void mod_io_setYellowLed( uint8_t val );

void ilb_scanButtons();

void ilb_setGreenLed( bool state );
void ilb_setRedLed( bool state );
void ilb_setYellowLed( bool state );


#endif /* IO_LED_BUTTON_H_ */