/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * twiMaster.c
 *
 * Created: 12/10/2020	0.01	ndp
 *  Author: Chip
 *
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "twiMaster.h"

// DEBUG
#include "mod_led.h"

/* *** Private functions *** */

/* *** Local Defines *** */
// General TWI Master status codes
typedef enum {
	TWI_IDLE,			// Waiting
	TWI_START,			// START has been transmitted
	TWI_REP_START,		// Repeated START has been transmitted
	TWI_ARB_LOST,		// Arbitration lost
	// TWI Master Transmitter status codes
	TWI_MTX_ADR,		// SLA+W has been transmitted
	TWI_MTX_ADR_ACK,	// SLA+W has been transmitted and ACK received
	TWI_MTX_ADR_NACK,	// SLA+W has been transmitted and NACK received
	TWI_MTX_DATA,		// Data byte has been transmitted
	TWI_MTX_DATA_ACK,	// Data byte has been transmitted and ACK received
	TWI_MTX_DATA_NACK,	// Data byte has been transmitted and NACK received
	// TWI Master Receiver status codes
	TWI_MRX_ADR,		// SLA+R has been transmitted
	TWI_MRX_ADR_ACK,	// SLA+R has been transmitted and ACK received
	TWI_MRX_ADR_NACK,	// SLA+R has been transmitted and NACK received
	TWI_MRX_DATA_ACK,	// Data byte has been received and ACK transmitted
	TWI_MRX_DATA_NACK,	// Data byte has been received and NACK transmitted
	// Error
	TWI_BUS_ERROR		// Bus error due to an illegal START or STOP condition
} TWI_MASTER_STATES;

/* *** Local variables *** */
#define TWI_MASTER_BUFF_SIZE	16

uint8_t txBuffer[TWI_MASTER_BUFF_SIZE];
uint8_t txBufferBytes;			// number of bytes to write.
uint8_t txBufferHead;
uint8_t txBufferTail;

//uint8_t rxBuffer[TWI_MASTER_BUFF_SIZE];
uint8_t* rxBuffer;
uint8_t rxBufferBytes;			// number of bytes to read.
uint8_t rxBufferHead;
uint8_t rxBufferTail;

volatile bool twiBusy = false;	// Stops isBusy tests if not initialize here.
uint8_t slaveAddress = 0;

volatile TWI_MASTER_STATES state;

/* *** Local Functions *** */

/*
 * Set up TWI hardware.
 *
 * I2C rate = fSCL = fCLK_PER / (10 + 2*100kHz)			fCLK_PER = 0.050 us, 20MHz PER
 */
void twiMasterInit()
{
	txBufferHead = txBufferTail = 0;
	rxBufferHead = rxBufferTail = 0;

	TWI0_MSTATUS = TWI_BUSSTATE_IDLE_gc;		// Force to IDLE state on reset.

	// Set BAUD rate to 100kHz
	TWI0_MBAUD = 82;

	TWI0_CTRLA = TWI_SDAHOLD_500NS_gc;		// SMBus conform.
	// Clear Master state, MADDR and MDATA.
	TWI0_MCTRLB = TWI_FLUSH_bm;
	// Read interrupt, Write interrupt, TimeOut 200us, Smart mode.
//	TWI0_MCTRLA = TWI_RIEN_bm | TWI_WIEN_bm | TWI_TIMEOUT_200US_gc | TWI_SMEN_bm;
	// Read interrupt, Write interrupt, TimeOut 200us, Smart mode, Enable.
	TWI0_MCTRLA = TWI_RIEN_bm | TWI_WIEN_bm | TWI_TIMEOUT_200US_gc | TWI_SMEN_bm | TWI_ENABLE_bm;
	state = TWI_IDLE;
	twiBusy = false;
}

/*
 * Send nbytes from buffer[] to Slave adrs.
 * Return false on failure or if busy.
 * adrs is 7-bit.
 */
bool twiWrite(uint8_t adrs, uint8_t* buffer, uint8_t nbytes)
{
	bool result = false;

	// Check for Error conditions.
	if(TWI0_MSTATUS & (TWI_CLKHOLD_bm | TWI_ARBLOST_bm | TWI_BUSERR_bm)) {
		// ERROR
		TWI0_MSTATUS |= TWI_CLKHOLD_bm | TWI_ARBLOST_bm | TWI_BUSERR_bm;
	}
//	mod_led_toggle(TWI0_MSTATUS);
	
	slaveAddress = adrs;		// save for recover.
	// if nbytes < TWI_MASTER_BUFF_SIZE, copy buffer[] into txBuffer, else Fail
	if(nbytes < TWI_MASTER_BUFF_SIZE) {
		for( int i=0; i<nbytes; i++ ) {
			txBuffer[i] = *buffer++;
		}
		result = true;
	}
	// Set number of bytes to send
	txBufferBytes = nbytes;
	// Reset indexes
	txBufferHead = txBufferTail = 0;
	// Set Slave address as SDA_W
	TWI0_MADDR = adrs<<1 & 0xFE;		// /W = 0

	state = TWI_MTX_ADR;
	twiBusy = true;
	
	return result;
}

/*
 * Return false if not, else true.
 */
bool twiIsBusy()
{
	return twiBusy;
}

/*
 * Read nbytes from Slave adrs into buffer[].
 */
bool twiRead(uint8_t adrs, uint8_t* buffer, uint8_t nbytes)
{
	bool result = false;

	if(nbytes != 0) {
		rxBuffer = buffer;
		rxBufferBytes = nbytes;
		// Reset indexes
		rxBufferHead = rxBufferTail = 0;
		// Set Slave address as SDA_W
		TWI0_MADDR = adrs<<1 | 1;		// R = 1
		if(nbytes == 1) {
			TWI0_MCTRLB = TWI_MCMD_STOP_gc;			// NACK last byte and issue STOP.
//			TWI0_MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;	// NACK last byte and issue STOP.
		} else {
			TWI0_MCTRLB = TWI_MCMD_RECVTRANS_gc;					// ACK next byte and continue.
		}
		state = TWI_MRX_ADR;
		twiBusy = true;
	}	
	
	return result;
}

/* *** Interrupt Service Routine *** */

/*
 * TWI Master Interrupt Service
 * Called by TWI Event
 */
ISR( TWI0_TWIM_vect )
{
//  mod_led_toggle(TWI0_MSTATUS & 0x07);
// mod_led_toggle(1);

	/* Decode the cause of the Master interrupt. */
	if( TWI0_MSTATUS & TWI_WIF_bm ) {
		// Check for Arbitration Loss or Buss Error
		if(TWI0_MSTATUS & (TWI_ARBLOST_bm | TWI_BUSERR_bm)) {
			if(TWI0_MSTATUS & TWI_ARBLOST_bm) {
				state = TWI_ARB_LOST;
			} else {
				state = TWI_BUS_ERROR;
			}
		} else {
			// Address/Data sent
			if( TWI0_MSTATUS & TWI_RXACK_bm ) {
				// No Slave ACKed address or Data was not ACKed.
				switch(state) {
					case TWI_MTX_ADR:
						// Was sending SDA_W address.
						state = TWI_MTX_ADR_NACK;
						break;
						
					case TWI_MRX_ADR:
						// Was sending SDA_R address
						state = TWI_MRX_ADR_NACK;
						break;
						
					case TWI_MTX_DATA:
						// Was sending data.
						state = TWI_MTX_DATA_NACK;
						break;
						
					default:
						// Oops
						state = TWI_BUS_ERROR;
						break;
				}
			} else {
				// Slave ACKed SDA_W address or data.
				if(state == TWI_MTX_ADR) {
					// Was sending address.
					state = TWI_MTX_ADR_ACK;
				} else {
					// Was sending data.
					state = TWI_MTX_DATA_ACK;
				}
			}
		}
	} else {
		// RIF caused interrupt. Was ACKed (SDA_R) or DATA read by Master.
		if(TWI0_MSTATUS & TWI_RXACK_bm) {
			// Master read DATA and Slave is done sending data. WIF catches NACKed SDA_R.
			state = TWI_MRX_DATA_NACK;
		} else {
			// SDA_R or data received.
			state = TWI_MRX_DATA_ACK;
		}
	}
	
	switch( state )
	{
		case TWI_IDLE:
 mod_led_toggle(7);
			break;

		case TWI_MTX_ADR_ACK:			// SLA+W has been transmitted and ACK received. Load DATA.
		case TWI_MTX_DATA_ACK:			// Data has been transmitted and ACK received. Load DATA.
			if(txBufferTail < txBufferBytes) {
				// Send next byte
				TWI0_MDATA = txBuffer[txBufferTail++];
			} else {
				// Send STOP after last byte. NOTE: May have to do this one byte earlier.
				TWI0_MCTRLB = TWI_MCMD_STOP_gc;
				twiBusy = false;
 mod_led_toggle(4);
			}
			break;
			
		case TWI_MRX_ADR:				// SLA+R has been transmitted and ACK received else WIF would get set.
			state = TWI_MRX_DATA_ACK;
		case TWI_MRX_DATA_ACK:			// Data byte has been received and ACK transmitted. Read Data.
//			rxBuffer[rxBufferHead++] = TWI0.MDATA;			// put here does not clear CLKHOLD.
			if(rxBufferHead > (rxBufferBytes-2)) {
				// Set to NACK next read then STOP.
				TWI0_MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
				twiBusy = false;
				rxBuffer[rxBufferHead++] = TWI0.MDATA;
 mod_led_toggle(5);
			} else {
				rxBuffer[rxBufferHead++] = TWI0.MDATA;		// placed after TWI0_MCTRLB causes data corruption.
				// Set to ACK and RECVTRANS.
				TWI0_MCTRLB = TWI_MCMD_RECVTRANS_gc;
			}
			break;

		// This case is NEVER entered. TWI_MRX_DATA_ACK ends with NACK and STOP.
		case TWI_MRX_DATA_NACK:				// Data byte has been received and NACK transmitted
			rxBuffer[rxBufferHead++] = TWI0.MDATA;		// Read last data byte. STOP would have been set.
			state = TWI_IDLE;
			twiBusy = false;
  			break;
			
		case TWI_MRX_ADR_NACK:
		case TWI_MTX_ADR_NACK:
		case TWI_MTX_DATA_NACK:
		case TWI_ARB_LOST:			// Arbitration lost
		case TWI_BUS_ERROR:			// Bus error
			state = TWI_IDLE;
		default:					// OOPS
			// Clear CLKHOLD, ARBLOST, and BUSERROR
			TWI0_MADDR = slaveAddress<<1;
			TWI0_MDATA = 0;
			TWI0_MCTRLB = TWI_MCMD_STOP_gc;
//			TWI0_MSTATUS = TWI_BUSSTATE_IDLE_gc;		// Force IDLE state.
			twiBusy = false;
 mod_led_toggle(8);
			break;
	}
}
