/*****************************************************************************
*
* Atmel Corporation
*
* File              : serial.c
* Compiler          : Atmel Studio 7.0
* Revision          : $Revision: 2.0 $
* Date              : $Date: 01/22/2017 $
* Updated by        : $Author: ndpearson (aka CmdrZin) $
*
* Support mail      : chip@gameactive.org
*
* Target platform   : All AVRs with bootloader support
*
* AppNote           : AVR112 - Self-programming
*
* Description       : UART communication routines
****************************************************************************/
#include <avr/io.h>

#define FOSC 20000000 // Clock Speed
#define BAUD 115200   //9600	//38400  //115200
#define MYUBRR ((FOSC/8/BAUD)-1)

void initbootuart()
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


void sendchar(char data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;

	/* Put data into buffer, sends the data */
	UDR0 = data;
}


char recchar(void)
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	
	/* Get and return received data from buffer */
	return UDR0;
}
