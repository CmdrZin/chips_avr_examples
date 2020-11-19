/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2020 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * File              : serialPoll.c
 * Compiler          : Atmel Studio 7.0
 * Revision          : $Revision: 2.0 $
 * Date              : $Date: 01/22/2017 $
 * Updated by        : $Author: ndpearson (aka CmdrZin) $
 * Revised           : $Date: 11/13/2020 $
 * Support mail      : chip@gameactive.org
 *
 * Target platform   : megaAVR 0-series
 *
 * Description       : Polling UART communication routines with blocking calls.
 *   For PC host through debugger: USART3 (PB0:Tx,PB1:Rx)
 *   For Arduino and other direct connections: USART0 (PA0:Tx, PA1:Rx)
 *
 * System clock: 20 MHz		Baud: 9600 8N1
 *
 * Code snippets	 : TB3216-Getting-Started-with-USART-90003216B.pdf Section 3.
 *
 ****************************************************************************/
#include <avr/io.h>

#include "serialPoll.h"

#define F_CPU 20000000L			// Clock Speed 20 MHz
#define USART_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64)/(16 * (float)BAUD_RATE) + 0.5)

/* Initialized USART0 registers and IO pins */
void USART0_init(uint16_t baud)
{
	/* Set IO port pin directions. */
	PORTA_DIR &= ~PIN1_bm;		// Rx as INPUT (default)
	PORTA_DIR |= PIN0_bm;		// Tx as OUTPUT
	
	/* Set baud rate */
	USART0_BAUD = (uint16_t)USART_BAUD_RATE(baud);

	/* Set frame format: 8bit Data, No Parity, 1stop bit (8N1) */
	USART0_CTRLC = USART_CHSIZE_8BIT_gc;	// default

	/* Enable receiver and transmitter */
	USART0_CTRLB |= (USART_RXEN_bm | USART_TXEN_bm);
}

/* Blocking call to send one character. */
void USART0_sendChar(char data)
{
	/* Wait for empty transmit buffer */
	while ( !(USART0_STATUS & USART_DREIF_bm ))	;

	/* Put data into buffer, sends the data */
	USART0_TXDATAL = data;
}

/* Blocking call to receive one character.
 * USART0_isChar() should be called and return 'true' before calling this function.
 */
char USART0_recvChar()
{
	/* Wait for data to be received */
	while ( !(USART0_STATUS & USART_RXCIF_bm) )	;
	
	/* Get and return received data from buffer */
	return USART0_RXDATAL;
}

// Check for character in Receive register
bool USART0_isChar()
{
	bool result = false;
	
	if( USART0_STATUS & USART_RXCIF_bm ) {
		result = true;
	}
	
	return( result );
}

/* *** USART3 functions *** */
/* Initialized USART3 registers and IO pins */
void USART3_init(uint16_t baud)
{
	/* Set IO port pin directions. */
	PORTB_DIR &= ~PIN1_bm;		// Rx as INPUT (default)
	PORTB_DIR |= PIN0_bm;		// Tx as OUTPUT
	
	/* Set baud rate */
	USART3_BAUD = (uint16_t)USART_BAUD_RATE(baud);

	/* Set frame format: 8bit Data, No Parity, 1stop bit (8N1) */
	USART3_CTRLC = USART_CHSIZE_8BIT_gc;	// default

	/* Enable receiver and transmitter */
	USART3_CTRLB |= (USART_RXEN_bm | USART_TXEN_bm);
}

/* Blocking call to send one character. */
void USART3_sendChar(char data)
{
	/* Wait for empty transmit buffer */
	while ( !(USART3_STATUS & USART_DREIF_bm ))	;

	/* Put data into buffer, sends the data */
	USART3_TXDATAL = data;
}

/* Blocking call to receive one character.
 * USART3_isChar() should be called and return 'true' before calling this function.
 */
char USART3_recvChar()
{
	/* Wait for data to be received */
	while ( !(USART3_STATUS & USART_RXCIF_bm) )	;
	
	/* Get and return received data from buffer */
	return USART3_RXDATAL;
}

// Check for character in Receive register
bool USART3_isChar()
{
	bool result = false;
	
	if( USART3_STATUS & USART_RXCIF_bm ) {
		result = true;
	}
	
	return( result );
}
