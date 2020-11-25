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
 * ArrayDataAccess
 *
 * Created: 11/1/2020 4:21:58 PM
 * Author : CmdrZin
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

const uint8_t prime[] PROGMEM = {1, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
const uint32_t noise[] PROGMEM = {0x11111111, 0x33333333, 0x55555555, 0x07070707,
	 0x0F0F0F0F, 0x1B1B1B1B};

int main(void)
{
	uint32_t* dptr;
	uint32_t data;
	
    while (1) 
    {
		// Access byte array by index
		for (int i = 0; i<sizeof(prime); i++) {
			DDRD = pgm_read_byte(&prime[i]);
		}
		
		// Access double word array by pointer adjust
		// NOTE the adjustment needed in the sizeof() calculation.
		dptr = noise;
		for (int i = 0; i<(sizeof(noise)/sizeof(uint32_t)); i++) {
			data = pgm_read_dword(dptr++);
			DDRD = data;
			DDRC = data>>8;
			DDRB = data>>16;
		}
    }
}
