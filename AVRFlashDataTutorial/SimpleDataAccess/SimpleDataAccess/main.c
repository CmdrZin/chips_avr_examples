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
 * SimpleDataAccess.c
 *
 * Created: 11/1/2020 4:21:58 PM
 * Author : CmdrZin
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

const uint8_t x PROGMEM = 5;
const uint16_t y PROGMEM = 0x1234;
const uint32_t z PROGMEM = 0x98765432;

const char mystring[] PROGMEM = "This is a string.";	// null terminated.

int main(void)
{
	int index;
	char data;
	
    /* Replace with your application code */
    while (1) 
    {
		DDRB = pgm_read_byte(&x);		// use a byte from FLASH
		DDRC = pgm_read_word(&y);		// use LSB byte of a word in FLASH
		DDRD = pgm_read_dword(&z)>>24;	// use MSB byte from 32bit in FLASH
		
		// Use string bytes.
		index = 0;
		while ( (data = pgm_read_byte(&mystring[index])) != 0 )
		{
			DDRD = data;
			++index;
		}		
    }
}
