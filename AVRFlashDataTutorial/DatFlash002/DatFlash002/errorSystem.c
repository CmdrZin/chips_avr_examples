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
 * errorSystem.c
 *
 * Created: 11/1/2020 6:25:38 PM
 *  Author: CmdrZin
 */ 
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "errorSystem.h"

void test1();
void test2();


// Error messages
const char errOverflow = "An overflow has occurred.";
const char errTimeout = "The current process has timed out.";
const char errPower = "A power anomaly has been detected.";

const ERROR_RESPONSE errors[] PROGMEM =
{
	{ ERROR_OVERFLOW, errOverflow, test1 },
	{ ERROR_POWER, errPower, test2 },
	{ ERROR_TIMEOUT, errTimeout, test1 },
	{ 0, 0, 0 }
};

/* Search table in FLASH for flag. */
ERROR_RESPONSE errorHandler(ERROR_FLAG flag)
{
	
	
}


// Error response functions to handle the error condition.
void test1() {
	PORTD = 0;
}

void test2() {
	PORTD = 7;
}
