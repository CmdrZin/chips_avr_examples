/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * sysTimer.h
 *
 * Created: 5/19/2015 1:06:23 PM
 *  Author: Chip
 * revision: 8/1/2015	0.01	ndp
 * Revised: 7/24/2017	0.10	ndp replace GPIO_TIC system with millis()
 * hack:	9/24/2018	0.12	ndp	use 4us tics for high resolution in RC signal decode.
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

long st_millis();
long getTics4us();

void st_init_tmr0();

#endif /* SYSTIMER_H_ */