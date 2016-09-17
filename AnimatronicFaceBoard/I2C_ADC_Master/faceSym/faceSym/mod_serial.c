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
 * mod_serial.c
 *
 * Created: 9/9/2015 1:36:10 PM
 *  Author: Chip
 * 
 * Example code from ATmega164P data sheet.
 */ 

#include <avr/interrupt.h>

#include "mod_serial.h"


#define FOSC 20000000 // Clock Speed
#define BAUD 38400	//9600	//38400  //115200
#define MYUBRR ((FOSC/8/BAUD)-1)

/*
 * Initialize Serial Port USART0
 */
void mod_serial_init()
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(MYUBRR>>8);
	UBRR0L = (unsigned char)MYUBRR;
	/* Double rate for finer baud resolution */
	UCSR0A = (1<<U2X0);
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8bit Data, No Parity, 1stop bit (8N1) */
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}

/*
 * Is Tx FIFO Empty?
 */
bool mser_fifoTxEmpty()
{
	return true;	
}

/*
 * Is Rx FIFO Empty?
 */
bool mser_fifoRxEmpty()
{
	return true;
}

/*
 * Trigger Serial to start writing out Tx FIFO
 */
void mod_serialWriteStart()
{
	
}

/*
 * Is Serial Port Busy sending data?
 */
bool mod_serialTxBusy()
{
	return false;
}

/*
 * Is there unread Rx FIFO data received by the Serial port?
 */
bool mod_serialRxHasData()
{
	return false;
}

/*
 * Get data from Rx FIFO
 */
uint8_t mser_fifoTxRead()
{
	return 0;	
}

/*
 * MANUAL: Send data
 * Example code. BLOCKING call.
 */
void mser_sendData( uint8_t data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
		;

	/* Put data into buffer, sends the data */
	UDR0 = data;
}

/*
 * MANUAL: Receive data
 * Example code. BLOCKING call.
 */
uint8_t mser_recvData()
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
		;
		
	/* Get and return received data from buffer */
	return UDR0;
}

ISR( USART0_RX_vect )
{
	
}

ISR( USART0_TX_vect )
{
	
}
