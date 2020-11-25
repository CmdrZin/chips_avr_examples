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
* MultipleDimensions
*
* Created: 11/1/2020 4:21:58 PM
* Author : CmdrZin
*/

#include <avr/io.h>
#include <avr/pgmspace.h>

const uint8_t waves[][8] PROGMEM = {
	{0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},
	{0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xFF},
	{0x00, 0x40, 0x80, 0xC0, 0xFF, 0xC0, 0x80, 0x40}
};

int main(void)
{
	while (1)
	{
		for(int j=0; j<(sizeof(waves)/sizeof(waves[0])); j++) {
			for (int i = 0; i<sizeof(waves[0]); i++) {
				DDRD = pgm_read_byte(&waves[j][i]);
			}
		}
	}
}
