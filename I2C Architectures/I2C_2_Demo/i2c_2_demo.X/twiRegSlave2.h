/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2026 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * twiRegSlave2.h
 *
 * Created: 08/10/2015	0.01	ndp
 *  Author: Chip
 * Revised: 09/24/2018	0.02	ndp	Support indexed read.
 * Revised: 11/11/2020	0.03	Update for megaAVR 0-series.
 * Revised: 05/13/2026  0.04    use rx/tx registers
 */ 


#ifndef TWIREGSLAVE_H_
#define TWIREGSLAVE_H_

#include <stdbool.h>

typedef struct {
    volatile void* rxBuff;          // can be modified by ISR
    volatile uint8_t  rxSize;       // can be modified by ISR
    void* txBuff;
    uint8_t  txSize;
} COMM_BUFF;

/* *** Global Prototypes *** */

void twiRegSlaveInit( uint8_t address, COMM_BUFF* rxtxBuffs, uint8_t rtSize);

#endif /* TWIREGSLAVE_H_ */