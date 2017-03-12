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
*
* File              : serial.c
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
#include <avr/io.h>

#include "serial.h"


#define FOSC 1000000L	// Clock Speed 1 MHz for low power
#define BAUD 4800L
#define MYUBRR ((FOSC/(16*BAUD))-1)

void initUsart()
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(MYUBRR>>8);
	UBRR0L = (unsigned char)MYUBRR;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8bit Data, No Parity, 1stop bit (8N1) */
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

	DDRD |= (1<<PORTD2);	// CTS output
	PORTD |= (1<<PORTD2);	// raise CTS
	DDRD &= ~(1<<PORTD3);	// DTR input
}


void sendChar(char data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;

	/* Put data into buffer, sends the data */
	UDR0 = data;
}


char recvChar()
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	
	/* Get and return received data from buffer */
	return UDR0;
}


// Check for character in Recv buffer
bool isChar()
{
	return( UCSR0A & (1<<RXC0) );
}