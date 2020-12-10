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
 * twiRegSlave.h
 *
 * Created: 08/10/2015	0.01	ndp
 *  Author: Chip
 * Revised: 09/24/2018	0.02	ndp	Support indexed read.
 * Revised: 11/11/2020	0.03	Update for megaAVR 0-series.
 */ 


#ifndef TWIREGSLAVE_CBWOPARAM_H_
#define TWIREGSLAVE_CBWOPARAM_H_

#include <stdbool.h>

/* *** GLobal Protoptyes *** */

void	twiRegSlaveInit( uint8_t address, 
			volatile uint8_t* rxRegFile, uint8_t rxRegFileSize, 
			volatile uint8_t* txRegFile, uint8_t txRegFileSize);

uint8_t* twiGetRegFilePtr();	// Return a pointer to the RegisterFile[0].

void	twiSetRegister( uint8_t reg, uint8_t val );		// called by sensor functions

uint8_t	twiGetRegister( uint8_t reg );		// get register value.

#endif /* TWIREGSLAVE_CBWOPARAM_H_ */