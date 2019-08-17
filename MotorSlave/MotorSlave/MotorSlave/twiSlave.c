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
 * twiSlave.c
 *
 * Created: 8/10/2015	0.01	ndp
 *  Author: Chip
 * Modified: 04/15/2019		0.20	ndp		Mods for Motor Slave project.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "twiSlave.h"

void flushTwiBuffers( void );

/* *** Local Defines *** */

// TWI Slave Receiver status codes
#define	TWI_SRX_ADR_ACK				0x60  // Own SLA+W has been received ACK has been returned
#define	TWI_SRX_ADR_ACK_M_ARB_LOST	0x68  // Own SLA+W has been received; ACK has been returned
#define	TWI_SRX_GEN_ACK				0x70  // General call address has been received; ACK has been returned
#define	TWI_SRX_GEN_ACK_M_ARB_LOST	0x78  // General call address has been received; ACK has been returned
#define	TWI_SRX_ADR_DATA_ACK		0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
#define	TWI_SRX_ADR_DATA_NACK		0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
#define	TWI_SRX_GEN_DATA_ACK		0x90  // Previously addressed with general call; data has been received; ACK has been returned
#define	TWI_SRX_GEN_DATA_NACK		0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
#define	TWI_SRX_STOP_RESTART		0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave

// TWI Slave Transmitter status codes
#define	TWI_STX_ADR_ACK				0xA8  // Own SLA+R has been received; ACK has been returned
#define	TWI_STX_ADR_ACK_M_ARB_LOST	0xB0  // Own SLA+R has been received; ACK has been returned
#define	TWI_STX_DATA_ACK			0xB8  // Data byte in TWDR has been transmitted; ACK has been received
#define	TWI_STX_DATA_NACK			0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
#define	TWI_STX_DATA_ACK_LAST_BYTE	0xC8  // Last byte in TWDR has been transmitted (TWEA = 0); ACK has been received

// TWI Miscellaneous status codes
#define	TWI_NO_STATE				0xF8  // No relevant state information available; TWINT = 0
#define	TWI_BUS_ERROR				0x00  // Bus error due to an illegal START or STOP condition

/* *** Local variables *** */
static uint8_t          rxBuf[ TWI_RX_BUFFER_SIZE ];
static volatile uint8_t rxHead;
static volatile uint8_t rxTail;

static uint8_t          txBuf[ TWI_TX_BUFFER_SIZE ];
static volatile uint8_t txHead;
static volatile uint8_t txTail;

/* *** Local Functions *** */
// Reset TWI buffers
void flushTwiBuffers( void )
{
	rxTail = 0;
	rxHead = 0;
	txTail = 0;
	txHead = 0;
}

/*
 * Set up TWI hardware and set Slave I2C Address.
 * TODO: Allow GLOBAL address.
 *
 * I2C rate = CPU/(16 + 2(TWBR)*(4^TWPS0:1)) = 20^6Hz / 200 = 100kHz
 *		20Mhz / 200 = 100kHz .. 16 + (2*92)*1) = 200
 *
 */
void twiSlaveInit( uint8_t adrs )
{
	TWBR = 92;			// set clock for 100kHz SCL
	TWSR = 0;			// pre-scale TWPS1:0 = 00
	
	TWAR = (adrs << 1)|(0);
	
	TWCR = (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
	return;
}

/*
 * Enable I2C Slave
 * NOTE: Could set status flags here also.
 */
void twiSlaveEnable( void )
{
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
}
	
// Place data into output buffer if there is available space.
void twiTransmitByte( uint8_t data )
{
	uint8_t tmphead;

	// calculate buffer index
	tmphead = ( txHead + 1 ) & TWI_TX_BUFFER_MASK;

	// check for free space in buffer
	if ( tmphead == txTail )
	{
		return;
	}

	// store data into buffer
	txBuf[ tmphead ] = data;

	// update index
	txHead = tmphead;
}

/*
 * Read data from input buffer.
 * Return 0x88 if no data is available. ERROR.
 */
uint8_t twiReceiveByte( void )
{
	// check for available data.
	if ( rxHead == rxTail )
	{
		return 0x88;
	}

	// generate index
	rxTail = ( rxTail + 1 ) & TWI_RX_BUFFER_MASK;

	return rxBuf[ rxTail ];
}

/*
 * Check for available data in input buffer. This function should return TRUE
 * before calling twiReceiveByte() to get data.
 */
bool twiDataInReceiveBuffer( void )
{
  // return 0 (false) if the receive buffer is empty
  return rxHead != rxTail;
}

// Reset the output buffer to empty. Used to recover from sync errors.
void twiClearOutput( void )
{
	txTail = 0;
	txHead = 0;
}

/*
 * Also used for manual input into input buffer for testing.
 * NOTE: If RX buffer is full, data is lost.
 * TODO: ERROR condition on data lost.
 */
void twiStuffRxBuf( uint8_t data )
{
	uint8_t tmphead;

	// calculate buffer index
	tmphead = ( rxHead + 1 ) & TWI_RX_BUFFER_MASK;

	// check for free space in buffer
	if ( tmphead == rxTail )
	{
		return;
	}

	// store data into buffer
	rxBuf[ tmphead ] = data;

	// update index
	rxHead = tmphead;
}

/* *** Interrupt Service Routines *** */

/*
 * TWI Interrupt Service
 * Called by TWI Event
 */
ISR( TWI_vect )
{
	switch( TWSR )
	{
		case TWI_SRX_ADR_ACK:		// 0x60 Own SLA+W has been received ACK has been returned. Expect to receive data.
//		case TWI_SRX_ADR_ACK_M_ARB_LOST:	// 0x68 Own SLA+W has been received; ACK has been returned. RESET interface.
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);		// Prepare for next event. Should be DATA.
			break;

		case TWI_SRX_ADR_DATA_ACK:	// 0x80 Previously addressed with own SLA+W; Data received; ACK'd
		case TWI_SRX_GEN_DATA_ACK:	// 0x90 Previously addressed with general call; Data received; ACK'd
			// Put data into RX buffer
			twiStuffRxBuf( TWDR );
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);		// Prepare for next event. Should be more DATA.
  			break;
			
		case TWI_SRX_GEN_ACK:				// 0x70 General call address has been received; ACK has been returned
//		case TWI_SRX_GEN_ACK_M_ARB_LOST:	// 0x78 General call address has been received; ACK has been returned
			// TODO: Set General Address flag
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);		// Prepare for next event. Should be DATA.
			break;

		case TWI_STX_ADR_ACK:				// 0xA8 Own SLA+R has been received; ACK has been returned. Load DATA.
//		case TWI_STX_ADR_ACK_M_ARB_LOST:	// 0xB0 Own SLA+R has been received; ACK has been returned
		case TWI_STX_DATA_ACK:				// 0xB8 Data byte in TWDR has been transmitted; ACK has been received. Load DATA.
			if ( txHead != txTail )
			{
				txTail = ( txTail + 1 ) & TWI_TX_BUFFER_MASK;
				TWDR = txBuf[ txTail ];
			}
			else
			{
				// the buffer is empty. Send 0x88
				TWDR = 0x88;
			}
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);		// Prepare for next event.
			break;

		case TWI_STX_DATA_NACK:		// 0xC0 Data byte in TWDR has been transmitted; NOT ACK has been received. End of Sending.
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);		// Prepare for next event. Should be new message.
			break;

		case TWI_SRX_STOP_RESTART:		// 0xA0 A STOP condition or repeated START condition has been received while still addressed as Slave
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);		// Prepare for next event.
			break;

		case TWI_SRX_ADR_DATA_NACK:			// 0x88 Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
		case TWI_SRX_GEN_DATA_NACK:			// 0x98 Previously addressed with general call; data has been received; NOT ACK has been returned
		case TWI_STX_DATA_ACK_LAST_BYTE:	// 0xC8 Last byte in TWDR has been transmitted (TWEA = 0); ACK has been received
		case TWI_NO_STATE:					// 0xF8 No relevant state information available; TWINT = 0
		case TWI_BUS_ERROR:					// 0x00 Bus error due to an illegal START or STOP condition
			TWCR =   (1<<TWSTO)|(1<<TWINT);   // Recover from TWI_BUS_ERROR
			// TODO: Set an ERROR flag to tell main to restart interface.
			break;

		default:					// OOPS
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);		// Prepare for next event. Should be more DATA.
			break;
	}
}