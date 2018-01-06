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
 * mod_system.h
 *
 * Created: 10/04/2016		0.01	ndp
 *  Author: Chip
 */ 


#ifndef mod_system_H_
#define mod_system_H_

#include <avr/io.h>

typedef enum { MSS_IDLE, MSS_MENU_INIT, MSS_MENU, MSS_MANUAL, MSS_PRGM, MSS_RUN, MSS_ERASE, MSS_STEP, MSS_FTIME } MSS_STATES;

void mod_system_init();
void mod_systey_setKeyFrame(uint8_t chan, uint8_t pos);
void mod_system_service();
MSS_STATES mod_system_getState();


#endif /* mod_system_H_ */