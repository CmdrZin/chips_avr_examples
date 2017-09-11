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
 */ 


#ifndef MOD_STDIO_H_
#define MOD_STDIO_H_

#include <avr/io.h>
#include <stdbool.h>

#define MOD_IO_GREEN_LED	0x10
#define MOD_IO_RED_LED		0x11
#define MOD_IO_YELLOW_LED	0x12

void mod_stdio_print( uint8_t line, char* buffer, uint8_t nbytes );
void mod_stdio_print2Hex( char* buff, uint8_t val1, uint8_t val2 );

void mod_stdio_led( uint8_t led, bool state );

void mod_stdio_bin2hex(char* buff, uint8_t val);

#endif /* MOD_STDIO_H_ */