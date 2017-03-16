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
 * usiTwiSlaveNI.h
 *
 * Created: 1/24/2017		0.01	ndp
 *  Author: Chip
 */ 


#ifndef USITWISLAVENI_H_
#define USITWISLAVENI_H_


typedef enum usiState {UTS_WAIT_FOR_START, UTS_WAIT_FOR_ADRS, UTS_WAIT_FOR_ADRS_ACK, UTS_WAIT_FOR_DATA_IN, 
						UTS_WAIT_FOR_DATA_OUT, UTS_WAIT_ON_ACK, UTS_WAIT_CHK_ACK, IDLE} UTS_STATE;

void uts_init(uint8_t adrs);
void uts_poll();
void uts_sendACK();
void uts_chkACK();

void clearRxFifo();
void clearTxFifo();
bool isRxEmpty();
bool isTxEmpty();
uint8_t getRxFifo();
void putRxFifo(uint8_t data);
void putTxFifo(uint8_t data);
uint8_t getTxFifo();


#endif /* USITWISLAVENI_H_ */