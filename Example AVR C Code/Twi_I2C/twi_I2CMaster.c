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
 * twi_I2CMaster.c
 *
 * Created: 8/27/2015	0.01	ndp
 *  Author: Chip
 * Revised:	01/26/2017	0.02	ndp		SDA_R bug fix and SDA_W data NAK fix.
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#include "twi_I2CMaster.h"


// General TWI Master status codes
#define TWI_START                  0x08  // START has been transmitted
#define TWI_REP_START              0x10  // Repeated START has been transmitted
#define TWI_ARB_LOST               0x38  // Arbitration lost

// TWI Master Transmitter status codes
#define TWI_MTX_ADR_ACK            0x18  // SLA+W has been transmitted and ACK received
#define TWI_MTX_ADR_NACK           0x20  // SLA+W has been transmitted and NACK received
#define TWI_MTX_DATA_ACK           0x28  // Data byte has been transmitted and ACK received
#define TWI_MTX_DATA_NACK          0x30  // Data byte has been transmitted and NACK received

// TWI Master Receiver status codes
#define TWI_MRX_ADR_ACK            0x40  // SLA+R has been transmitted and ACK received
#define TWI_MRX_ADR_NACK           0x48  // SLA+R has been transmitted and NACK received
#define TWI_MRX_DATA_ACK           0x50  // Data byte has been received and ACK transmitted
#define TWI_MRX_DATA_NACK          0x58  // Data byte has been received and NACK transmitted

#define TWI_BUS_ERROR              0x00  // Bus error due to an illegal START or STOP condition


#define TWI_MASTER_BUFF_SIZE	16

uint8_t timTxBuffer[TWI_MASTER_BUFF_SIZE];
uint8_t timTxBufferBytes;
uint8_t timTxBufferPtr;

uint8_t timRxBuffer[TWI_MASTER_BUFF_SIZE];
uint8_t timRxBufferBytes;
uint8_t timRxBufferPtr;

volatile bool	timBusy;

void tim_init()
{
	TWBR = 90;				// 90=20MHz->100k, 32=8MHz->100kHz, 2 8MHz->400kHz
//	TWBR = 2;				// 32=8MHz->100kHz, 2 8MHz->400kHz
	timBusy = false;
// DEBUG++
//	DDRD |= (1<<PORTD0);	// set as OUTPUT
//	PORTD &= ~(1<<PORTD0);	// set LOW
// DEBUG--
}

void tim_write( uint8_t sla, uint8_t* buffer, uint8_t nbytes )
{
	// Copy address to buffer
	timTxBuffer[0] = (sla<<1);
	// Copy data to local buffer.
	timTxBufferBytes = nbytes + 1;
	for( uint8_t i = 1; i<timTxBufferBytes; i++ )
	{
		timTxBuffer[i] = buffer[i-1];
	}	
	
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);		// Initiate a START condition.
	
	timBusy = true;
// DEBUG++
//	PORTD |= (1<<PORTD0);	// set HIGH
// DEBUG--
}

bool tim_isBusy()
{
	return( timBusy );
}

void tim_read( uint8_t sla, uint8_t nbytes )
{
	// Copy address to buffer
	timTxBuffer[0] = (sla<<1)|(1);
	timTxBufferBytes = 1;							// just send adrs
	// Copy number of expected bytes.
	timRxBufferBytes = nbytes;

	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);		// Initiate a START condition.

	timBusy = true;
// DEBUG++
//	PORTD |= (1<<PORTD0);	// set HIGH
// DEBUG--
}

bool tim_hasData()
{
	return (timRxBufferPtr > 0);
}


uint8_t tim_readData()
{
	if (timRxBufferPtr > 0)
	{
		--timRxBufferPtr;
		return (timRxBuffer[ timRxBufferPtr ]);
	}
	else
	{
		return 0x88;
	}
}


ISR( TWI_vect )
{
	switch( TWSR )
	{
		case TWI_START:						// START has been transmitted
		case TWI_REP_START:					// Repeated START has been transmitted
			timTxBufferPtr = 0;				// Set buffer pointer to the TWI Address location
		case TWI_MTX_ADR_ACK:				// SLA+W has been transmitted and ACK received
		case TWI_MTX_DATA_ACK:				// Data byte has been transmitted and ACK received
			if (timTxBufferPtr < timTxBufferBytes)
			{
				TWDR = timTxBuffer[timTxBufferPtr++];
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);
			}
			else							// Send STOP after last byte
			{
				TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);	// Disable TWI Interrupt and clear the flag
														// Initiate a STOP condition.
				timBusy = false;
// DEBUG++
//				PORTD &= ~(1<<PORTD0);	// set LOW
// DEBUG--
			}
			break;

		case TWI_MRX_ADR_ACK:				// SLA+R has been transmitted and ACK received
			timRxBufferPtr = 0;				// Set buffer pointer start
			if ( (timRxBufferBytes == 0 ) || (timRxBufferPtr < (timRxBufferBytes-1)) )		// Check for zero read or last byte to read.
			{
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);		// Set to recv data.
			}
			else
			{
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);		// NACK
// DEBUG++
//				PORTD &= ~(1<<PORTD0);	// set LOW
// DEBUG--
			}
			break;
			
		case TWI_MRX_DATA_ACK:				// Data byte has been received and ACK transmitted
		    timRxBuffer[timRxBufferPtr] = TWDR;
			++timRxBufferPtr;
			if (timRxBufferPtr < (timRxBufferBytes-1) )		// Detect the last byte to NACK it.
			{
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);		// Set to recv data.
		    }
			else
			{
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);		// NACK
// DEBUG++
//				PORTD &= ~(1<<PORTD0);	// set LOW
// DEBUG--
			}
			break;
			
		case TWI_MRX_DATA_NACK:						// Data byte has been received and NACK transmitted
			timRxBuffer[timRxBufferPtr++] = TWDR;
			TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);	// Initiate a STOP condition.
			timBusy = false;
// DEBUG++
//			PORTD &= ~(1<<PORTD0);	// set LOW
// DEBUG--
			break;
			
		case TWI_ARB_LOST:						// Arbitration lost
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);	// Initiate a (RE)START condition.
			timBusy = false;
// DEBUG++
//			PORTD &= ~(1<<PORTD0);	// set LOW
// DEBUG--
		    break;
			
		case TWI_MTX_ADR_NACK:		// SLA+W has been transmitted and NACK received
		case TWI_MRX_ADR_NACK:		// SLA+R has been transmitted and NACK received
		case TWI_MTX_DATA_NACK:		// Data byte has been transmitted and NACK received
		case TWI_BUS_ERROR:			// Bus error due to an illegal START or STOP condition
		default:
			TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);	// Initiate a STOP condition. Try this..Much better.
			timBusy = false;
// DEBUG++
//			PORTD &= ~(1<<PORTD0);	// set LOW
// DEBUG--
			break;
	}
}
