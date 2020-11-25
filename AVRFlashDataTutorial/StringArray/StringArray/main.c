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
 * StringArray
 *
 * Created: 11/1/2020 4:21:58 PM
 * Author : CmdrZin
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

// These are arrays of char.
const char string0[] PROGMEM = "This is a string1.";	// null terminated.
const char string1[] PROGMEM = "A string2.";			// null terminated.
const char string2[] PROGMEM = "And a string3.";		// null terminated.
// This is an array of pointers.
// const char *strings[3] PROGMEM = { string0, string1, string2 };  // compiler error

// Make a struct to hold the pointer.
typedef struct {
	const char* strPtr;
} STRING_PTR;
// then make an array of structs.
const STRING_PTR strings[] PROGMEM =
{
	{ string0 },
	{ string1 },
	{ string2 }
};

int main(void)
{
	char data;
	char* sptr;
	
    while (1) 
    {
		// Pointer has to be cast to the proper type.
		sptr = (char*)pgm_read_ptr(&strings[1].strPtr);
		while ( (data = pgm_read_byte(sptr++)) != 0 )
		{
			DDRD = data;
		}		
    }
}
