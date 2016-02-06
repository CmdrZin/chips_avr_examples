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
 */

/*
 * sysTimer.h
 *
 * Created: 5/19/2015 1:06:23 PM
 *  Author: Chip
 * revision: 8/1/2015	0.01	ndp
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

#include <avr/io.h>

// 1ms tic flags
#define DEV_1MS_TIC		0			// Device service tic
//#define				1
//#define				2
//#define				3
// 10ms tic flags
#define	DEV_10MS_TIC	4
//#define				5
//#define				6
//#define				7

void st_init_tmr0();

void st_init_tmr2();
uint8_t tmr2_getCount();
void tmr2_clrCount();
void st_tmr2_clr();


#endif /* SYSTIMER_H_ */