/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2026 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * twiRegSlave2.c
 *
 * Created: 08/10/2015	0.01	ndp
 *  Author: Chip
 * Revised: 09/24/2018	0.02	ndp	Use register base read.
 * Revised: 11/10/2020	0.03	ndp	Port to ATmega4808/4809.
 * Revised: 04/15/2026	0.04	ndp	Port to Motor Slave, Attiny814, remove callbacks.
 * Revised: 05/11/2026  0.05    ndp Use rx/tx register table.
 *
 * First byte after SDA_W is the register index for writing or reading.
 * Successive reads increment the tx register index.
 * Multiple byte writes increment the rx register index on each byte after the first.
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "twiRegSlave2.h"

/* *** Local Defines *** */
typedef enum {
	TWI_STX_ADR_ACK,		// SDA+R valid address with ACK
	TWI_SRX_ADR_ACK,		// SDA+W valid address with ACK 
	TWI_STX_DATA_ACK,		// SDA+R valid data sent with ACK
	TWI_SRX_DATA_ACK,		// SDA+W valid data with ACK 
	TWI_STX_DATA_NACK,		// SDA+R valid data sent with NACK, Last byte
	TWI_BUS_ERROR,			// Collision or other Bus error.
	TWI_STOP				// STOP condition
} TWI_SLAVE_STATES;

/* *** Local variables *** */
static COMM_BUFF* rxtxBuffers;
static uint8_t rxtxSize;

static volatile uint8_t* rxRegister;        // set by ISR
static volatile uint8_t* txRegister;        // set by ISR

static volatile uint8_t	rxRegSize;          // set by ISR
static volatile uint8_t	txRegSize;          // set by ISR

static volatile uint8_t	rxRegIndex;         // set by ISR
static volatile uint8_t	txRegIndex;         // set by ISR

/*
 * Use of txRegister as a register pointer.
 * txRegisters[0] = 0x00 	should always by 0 for I2C scans
 * txRegisters are loaded by other functions.
 *
 * Use of rxRegister as a register pointer.
 * rxRegisters store data received from Master.
 */

/*
 * Set up TWI hardware and set Slave I2C Address.
 *
 * I2C rate = CPU/(16 + 2(TWBR)*(4^TWPS0:1)) = 20^6Hz / 200 = 100kHz
 *		20Mhz / 200 = 100kHz .. 16 + (2*92)*1) = 200
 *
 */
void twiRegSlaveInit( uint8_t adrs, COMM_BUFF* rxtxBuffs, uint8_t rtSize )
{
    rxtxBuffers = rxtxBuffs;
    rxtxSize = rtSize;

	TWI0_CTRLA = TWI_SDAHOLD_500NS_gc;	// SMBus conform.
	// Data interrupt, Address/Stop interrupt, Use address match, Smart mode, TWI Enabled.
	TWI0_SCTRLA = TWI_DIEN_bm | TWI_APIEN_bm | TWI_PIEN_bm | TWI_SMEN_bm | TWI_ENABLE_bm;
	TWI0_SADDR = (adrs << 1)|(0);		// Set slave address.
}

/* *** Interrupt Service Routines *** */
/*
 * TWI Slave Interrupt Service
 * Called by TWI Event
 */
ISR( TWI0_TWIS_vect )
{
    static uint8_t wkReg = 0;           // working register for current message.
	static bool setRegIndex = false;	// true: next byte is tx register index
	
	TWI_SLAVE_STATES	state;

	/* Decode the cause of the Slave interrupt. */
	if( TWI0_SSTATUS & TWI_APIF_bm ) {
		// Address/Stop interrupt (APIF)
		if( TWI0_SSTATUS & TWI_AP_bm ) {
			// Valid address
			if( TWI0_SSTATUS & TWI_DIR_bm ) {
				state = TWI_STX_ADR_ACK;			// [1] Master Read (tx)
			} else {
				state = TWI_SRX_ADR_ACK;			// [0] Master Write (rx)
			}
		} else {
			// STOP condition
			state = TWI_STOP;
		}
	} else if( TWI0_SSTATUS & TWI_DIF_bm ) {
		// Data transfer interrupt (DIF)
		if( TWI0_SSTATUS & TWI_DIR_bm ) {
			if( TWI0_SSTATUS & TWI_RXACK_bm ) {
				// Works this way though.
				state = TWI_STX_DATA_ACK;			// [1] Master Read (tx), Last Read byte NACK, NOT Write byte.
			} else {
				state = TWI_STX_DATA_ACK;			// [1] Master Read (tx)
			}
		} else {
			state = TWI_SRX_DATA_ACK;				// [0] Master Write (rx)
		}
	} else {
		state = TWI_BUS_ERROR;
	}
	// Override on Error
	if( TWI0_SSTATUS & (TWI_COLL_bm | TWI_BUSERR_bm) ) {
		state = TWI_BUS_ERROR;
	}
	
	switch( state )
	{
		case TWI_STX_ADR_ACK:		// [R] Own SLA+R has been received; ACK has been returned. Load DATA.
			TWI0_SCTRLB = TWI_SCMD_RESPONSE_gc;	// generates a data interrupt. clock stretch active.
			break;
			
		case TWI_STX_DATA_ACK:		// [R] Data byte in TWDR has been transmitted; ACK has been received. Load DATA.
			// Block overflow read.
			if( txRegIndex < txRegSize ) {
				TWI0_SDATA = txRegister[txRegIndex++];		// load data and advance index.
			} else {
                TWI0_SDATA = 0xFF;                          // dummy data. no more valid data.
            }
			TWI0_SCTRLB = TWI_SCMD_RESPONSE_gc;
			break;

		case TWI_SRX_ADR_ACK:		// [W] Own SLA+W has been received ACK has been returned.
			setRegIndex = true;		// Next byte is register value.
			TWI0_SCTRLB = TWI_SCMD_RESPONSE_gc;
			break;

		case TWI_SRX_DATA_ACK:		// Previously addressed with own SLA+W; Data received; ACK'd
			if(setRegIndex) {
                wkReg = TWI0_SDATA;         // First byte after SDA+W is register index.
                if( wkReg > rxtxSize ) {
                    // default to 0 Register for invalid values.
                    wkReg = 0;
                }
                // Index rxtxBuffers table for pointer to buffers and size limits.
                rxRegister = rxtxBuffers[wkReg].rxBuff;
                rxRegSize = rxtxBuffers[wkReg].rxSize;
                txRegister = rxtxBuffers[wkReg].txBuff;
                txRegSize = rxtxBuffers[wkReg].txSize;
                rxRegIndex = 0;
                txRegIndex = 0;
				setRegIndex = false;		// clear flag. Next write is data.
			} else {
				// Block overflow write.
				if( rxRegIndex < rxRegSize ) {
					rxRegister[rxRegIndex++] = TWI0_SDATA;		// Save data into RX buffer
                    // DEBUG
                    rxtxSize = 10;
				}
			}
			TWI0_SCTRLB = TWI_SCMD_RESPONSE_gc;
  			break;
			
		case TWI_STX_DATA_NACK:				// Data byte sent; Master NACK. End of Sending.
			TWI0_SCTRLB = TWI_SCMD_COMPTRANS_gc;	// Prepare for next event. Should be new message.
			break;

		case TWI_STOP:			// STOP condition or repeated START condition has been received while still addressed as Slave
			TWI0_SCTRLB = TWI_SCMD_COMPTRANS_gc;	// Prepare for next event. Should be new message.
			break;
			
		case TWI_BUS_ERROR:		// Bus error
			TWI0_SCTRLB = TWI_SCMD_COMPTRANS_gc;	// Prepare for next event. Should be new message.
			break;

		default:					// OOPS
			TWI0_SCTRLB = TWI_SCMD_COMPTRANS_gc;	// Prepare for next event. Should be new message.
			break;
	}
}
