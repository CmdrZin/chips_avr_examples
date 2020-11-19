/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
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
* File              : serialPoll.h
* Compiler          : Atmel Studio 7.0
* Revision          : $Revision: 2.0 $
* Date              : $Date: 01/22/2017 $
* Updated by        : $Author: ndpearson (aka CmdrZin) $
*
* Support mail      : chip@gameactive.org
*
* Target platform   : All AVRs with USART support
*
* Description       : UART communication routines
****************************************************************************/
#ifndef SERIALPOLL_H_
#define SERIALPOLL_H_

#include <stdbool.h>

void USART0_init(uint16_t baud);
void USART0_sendChar(char data);
char USART0_recvChar();
bool USART0_isChar();

void USART3_init(uint16_t baud);
void USART3_sendChar(char data);
char USART3_recvChar();
bool USART3_isChar();


#endif /* SERIALPOLL_H_ */
