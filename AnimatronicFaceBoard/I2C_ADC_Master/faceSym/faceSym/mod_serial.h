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
 * mod_serial.h
 *
 * Created: 9/9/2015 1:36:19 PM
 *  Author: Chip
 *
 * Baud: 38.4k, 8N1
 * Serial port functions.
 */ 


#ifndef MOD_SERIAL_H_
#define MOD_SERIAL_H_

#include <avr/io.h>
#include <stdbool.h>


void mod_serial_init();
bool mser_fifoTxEmpty();
bool mser_fifoRxEmpty();
void mod_serialWriteStart();
bool mod_serialTxBusy();
bool mod_serialRxHasData();
uint8_t mser_fifoTxRead();

void mser_sendData( uint8_t data );
uint8_t mser_recvData();


#endif /* MOD_SERIAL_H_ */