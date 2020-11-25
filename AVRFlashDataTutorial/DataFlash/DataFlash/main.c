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
 * DataFlash 001
 *
 * Created: 11/1/2020 4:21:58 PM
 * Author : CmdrZin
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

void test();

const uint8_t x PROGMEM = 5;
const uint16_t y PROGMEM = 0x1234;
const uint32_t z PROGMEM = 0x98765432;
const uint8_t w PROGMEM = 7;

//const void (*func)() PROGMEM = test;			// This generates a compiler error.
//const void (*afunc)()[] PROGMEM = {test};		// This also generates a compiler error.

/* +++ This does not generate a compiler error. +++ */
typedef struct
{
	void (*func)();
} FUNCTION_PTR;

const FUNCTION_PTR pfunc[] PROGMEM =
{
	{ test }
};
/* --- */

int main(void)
{
	void (*call)();
	
    /* Replace with your application code */
    while (1) 
    {
		PORTB = pgm_read_byte(&x);					// output a byte from FLASH
		PORTC = pgm_read_word(&y);					// output a byte of a word in FLASH
		PORTD = pgm_read_dword(&z);					// output a byte from 32bit in FLASH
		call = (void (*)())pgm_read_ptr(&pfunc[0]);	// call a function pointer stored in FLASH
		call();
		PORTB = pgm_read_byte(&w);					// output a byte from FLASH
    }
}

void test() {
	PORTD = 0;
}
