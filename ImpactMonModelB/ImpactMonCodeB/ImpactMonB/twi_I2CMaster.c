/*
 * twi_I2CMaster.c
 *
 * Created: 8/27/2015 3:30:32 PM
 *  Author: Chip
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


#define TWI_MASTER_BUFF_SIZE	8

uint8_t timTxBuffer[TWI_MASTER_BUFF_SIZE];
uint8_t timTxBufferBytes;
uint8_t timTxBufferPtr;

uint8_t timRxBuffer[TWI_MASTER_BUFF_SIZE];
uint8_t timRxBufferBytes;
uint8_t timRxBufferPtr;

bool	timBusy;

void tim_init()
{
	TWBR = 32;				// 32=8MHz->100kHz, 2 8MHz->400kHz
	timBusy = false;
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
			}
			break;

		case TWI_MRX_ADR_ACK:				// SLA+R has been transmitted and ACK received
			timRxBufferPtr = 0;				// Set buffer pointer start
			if (timRxBufferPtr < timRxBufferBytes )		// Check for zero read.
			{
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);
			}
			else
			{
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);		// NACK
			}
			break;
			
		case TWI_MRX_DATA_ACK:				// Data byte has been received and ACK traNsmitted
		    timRxBuffer[timRxBufferPtr++] = TWDR;
			if (timRxBufferPtr < timRxBufferBytes-1 )		// Detect the last byte to NACK it.
			{
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);
		    }
			else
			{
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);		// NACK
			}
			break;
			
		case TWI_MRX_DATA_NACK:						// Data byte has been received and NACK transmitted
			timRxBuffer[timRxBufferPtr++] = TWDR;
			TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);	// Initiate a STOP condition.
			timBusy = false;
			break;
			
		case TWI_ARB_LOST:						// Arbitration lost
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);	// Initiate a (RE)START condition.
		    break;
			
		case TWI_MTX_ADR_NACK:		// SLA+W has been transmitted and NACK received
		case TWI_MRX_ADR_NACK:		// SLA+R has been transmitted and NACK received
		case TWI_MTX_DATA_NACK:		// Data byte has been transmitted and NACK received
		case TWI_BUS_ERROR:			// Bus error due to an illegal START or STOP condition
		default:
		    // Reset TWI Interface
			TWCR = (1<<TWEN);
			timBusy = false;
	}
}
