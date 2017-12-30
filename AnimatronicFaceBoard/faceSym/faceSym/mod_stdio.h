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
 * mod_stdio.h
 *
 * Created: 7/23/2016 10:32:15 PM
 *  Author: Chip
 * Revision: 12/25/2017		0.01	ndp	update to new LCD API
 */ 


#ifndef MOD_STDIO_H_
#define MOD_STDIO_H_

#include <avr/io.h>
#include <stdbool.h>

#define LCD_CTRL				0x00
#define LCD_GET_BUTTONS			0x01
#define LCD_GET_BUTTONS_RAW		0x02
#define LCD_GET_ADC				0x03
#define LCD_TEXT				0x10
#define LCD_PTEXT				0x11
#define LCD_SET_GREEN_LED		0x20
#define LCD_SET_RED_LED			0x21
#define LCD_SET_YELLOW_LED		0x22

void mod_stdio_print( char* buffer, uint8_t nbytes );
void mod_stdio_printOffset( char* buffer, uint8_t nbytes, uint8_t offset );

void mod_stdio_led( uint8_t led, bool state );

#endif /* MOD_STDIO_H_ */