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
 * DataFlash 002
 *
 * Created: 11/1/2020 4:21:58 PM
 * Author : CmdrZin
 *
 * This example shows a complex Error Response system. It uses the error flag to look up 
 * the error response using a linear search.
 * A data structure contains the following for each error flag (see ERROR_RESPONSE):
 * 1. The error flag from an enum set.
 * 2. The error message string, null terminated.
 * 3. A function to call to respond to the error.
 *
 * A call to errorHandler() passed the following:
 * 1. The error flag.
 * The errorHandler returns an ERROR_RESPONSE struct or null if the error flag is not found.
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "errorSystem.h"

void serialDump(char* dataPtr);

int main(void)
{
	int count = 0;
	ERROR_RESPONSE errorInfo;
	
    /* Simple test code */
    while (1) 
    {
		if(count % 3 == 0) {
			errorInfo = errorHandler(ERROR_TIMEOUT);
			serialDump(errorInfo.msg);
			errorInfo.func();
		}

		if(count % 3 == 1) {
			errorInfo = errorHandler(ERROR_OVERFLOW);
			serialDump(errorInfo.msg);
			errorInfo.func();
		}
    }
}

void serialDump(char* dataPtr)
{
	while( *dataPtr != 0)
	{
		PORTC = *dataPtr++;
	}
}