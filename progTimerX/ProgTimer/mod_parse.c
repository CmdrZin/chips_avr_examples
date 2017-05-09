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
 * mod_parse.c
 *
 * Created: 2/2/2017 11:29:26 AM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "mod_parse.h"

struct tm	myTm;

 /* Parse Event time string (19 char+NULL) YYYY-MM-DD HH:MM:SS0
  * 2017-09-23 12:34:56
  */
time_t mod_parse_event(char* str)
 {
	char*		strPtr;
	uint8_t		data;

	strPtr = str;

	// get year
	if(*strPtr == 0) return 0;
	data = (*strPtr++ - '0') * 10;
	if(*strPtr == 0) return 0;
	data += (*strPtr++ - '0');
	data -= 19;
	if(*strPtr == 0) return 0;
	data = (data * 100) + ((*strPtr++ - '0') * 10);
	if(*strPtr == 0) return 0;
	myTm.tm_year = data + (*strPtr++ - '0');			// YYYY - 1900
	if(*strPtr++ != '-') return 0;

	// get month 0-11
	if(*strPtr == 0) return 0;
	data = (*strPtr++ - '0') * 10;
	if(*strPtr == 0) return 0;
	myTm.tm_mon = data + (*strPtr++ - '0') - 1;
	if((*strPtr++) != '-') return 0;

	// get day 1-31
	if(*strPtr == 0) return 0;
	data = (*strPtr++ - '0') * 10;
	if(*strPtr == 0) return 0;
	myTm.tm_mday = data + (*strPtr++ - '0');
	if((*strPtr++) != ' ') return 0;

	// get hour
	if(*strPtr == 0) return 0;
	data = (*strPtr++ - '0') * 10;
	if(*strPtr == 0) return 0;
	myTm.tm_hour = data + (*strPtr++ - '0');
	if(*strPtr++ != ':') return 0;

	// get minutes
	if(*strPtr == 0) return 0;
	data = (*strPtr++ - '0') * 10;
	if(*strPtr == 0) return 0;
	myTm.tm_min = data + (*strPtr++ - '0');
	if(*strPtr++ != ':') return 0;

	// get seconds
	if(*strPtr == 0) return 0;
	data = (*strPtr++ - '0') * 10;
	if(*strPtr == 0) return 0;
	myTm.tm_sec = data + (*strPtr++ - '0');

 	return mktime(&myTm);
 }
