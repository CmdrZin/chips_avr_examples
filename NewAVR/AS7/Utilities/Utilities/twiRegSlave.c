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
 * twiSlave.c
 *
 * Created: 08/10/2015	0.01	ndp
 *  Author: Chip
 * Revised: 09/24/2018	0.02	ndp	Use register base read.
 * Revised: 11/10/2020	0.03	ndp	Port to ATmega4808/4809.
 *
 * First byte after SDA_W is the register index for writing or reading.
 * Successive reads increment the tx register index.
 * Multiple byte writes increment the rx register index on each byte after the first.
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "twiRegSlave.h"

// DEBUG
#include "mod_led.h"

/* *** Private functions *** */
void twiSetRxRegisterIndex( uint8_t val );
void twiSetTxRegisterIndex( uint8_t val );

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
static volatile uint8_t* rxRegisters;
static volatile uint8_t* txRegisters;

static uint8_t	rxRegSize;
static uint8_t	txRegSize;

static volatile uint8_t	rxRegIndex;
static volatile uint8_t	txRegIndex;

/*
 * Use of txRegisters[] as a register list.
 * txRegisters[0] = 0x00 	always
 * txRegisters[1] = ADCH	Temperature (default) Left shifted..upper 8 bits
 * txRegisters[2] = ADCL	Lower 2 bits
 *  ...
 * txRegisters[txRegSize-1]
 * txRegisters are loaded from other functions.
 *
 * Use of rxRegisters[] as command registers.
 * rxRegisters[0] = STATUS LED state. 0=OFF, 1=ON
 * rxRegisters[1] = ADC Sample Rate H (default)
 * rxRegisters[2] = ADC Sample Rate L
 *  ...
 * rxRegisters[rxRegSize-1]
 * rxRegisters are loaded from data received from Master.
 */

/* *** Local Functions *** */
void twiSetRxRegisterIndex( uint8_t val )
{
	rxRegIndex = val;
}

void twiSetTxRegisterIndex( uint8_t val )
{
	txRegIndex = val;
}

/*
 * Set up TWI hardware and set Slave I2C Address.
 * TODO: Allow GLOBAL address.
 *
 * I2C rate = CPU/(16 + 2(TWBR)*(4^TWPS0:1)) = 20^6Hz / 200 = 100kHz
 *		20Mhz / 200 = 100kHz .. 16 + (2*92)*1) = 200
 *
 */
void twiRegSlaveInit( uint8_t adrs, volatile uint8_t* rxReg, uint8_t rxSize,
						 volatile uint8_t* txReg, uint8_t txSize)
{
	rxRegisters = rxReg;
	rxRegSize = rxSize;
	txRegisters = txReg;
	txRegSize = txSize;

	txRegisters[0] = 0;					// zero default register.

	TWI0_CTRLA = TWI_SDAHOLD_500NS_gc;	// SMBus conform.
	// Data interrupt, Address/Stop interrupt, Use address match, Smart mode, TWI Enabled.
	TWI0_SCTRLA = TWI_DIEN_bm | TWI_APIEN_bm | TWI_PIEN_bm | TWI_SMEN_bm | TWI_ENABLE_bm;
	TWI0_SADDR = (adrs << 1)|(0);		// Set slave address.
}

// Write to tx register file. Block overflow indexes.
void twiSetRegister( uint8_t reg, uint8_t val )
{
	if( reg < txRegSize ) {
		txRegisters[reg] = val;
	}
}

// Read from rx register file. Overflow indexes return 0x88.
uint8_t twiGetRegister( uint8_t reg )
{
	uint8_t result = 0x88;
	
	if( reg < txRegSize ) {
		result = rxRegisters[reg];
	}
	
	return result;
}

/* *** Interrupt Service Routines *** */

/*
 * TWI Slave Interrupt Service
 * Called by TWI Event
 */
ISR( TWI0_TWIS_vect )
{
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
				// BUG..looks at last READ transfer, not last data transfer.
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
// mod_led_toggle(9);
			break;
			
		case TWI_STX_DATA_ACK:		// [R] Data byte in TWDR has been transmitted; ACK has been received. Load DATA.
			// Block overflow read.
			if( txRegIndex < txRegSize ) {
				TWI0_SDATA = txRegisters[txRegIndex++];		// load data and advance index.
			}
			TWI0_SCTRLB = TWI_SCMD_RESPONSE_gc;
// mod_led_toggle(4);
			break;

		case TWI_SRX_ADR_ACK:		// [W] Own SLA+W has been received ACK has been returned.
			setRegIndex = true;		// Next byte is starting register.
			TWI0_SCTRLB = TWI_SCMD_RESPONSE_gc;
// mod_led_toggle(1);
			break;

		case TWI_SRX_DATA_ACK:		// Previously addressed with own SLA+W; Data received; ACK'd
			if(setRegIndex) {
				txRegIndex = TWI0_SDATA;	// First byte after SDA+W is register index.
				rxRegIndex = txRegIndex;	// Use only one reg read. First byte after SDA+W is register index.
				setRegIndex = false;		// clear flag
// mod_led_toggle(10);
			} else {
				// Block overflow write.
				if( rxRegIndex < rxRegSize ) {
					rxRegisters[rxRegIndex++] = TWI0_SDATA;		// Save data into RX buffer
				}
// mod_led_toggle(2);
			}
			TWI0_SCTRLB = TWI_SCMD_RESPONSE_gc;
  			break;
			
		case TWI_STX_DATA_NACK:				// Data byte sent; Master NACK. End of Sending.
			TWI0_SCTRLB = TWI_SCMD_COMPTRANS_gc;	// Prepare for next event. Should be new message.
// mod_led_toggle(5);
			break;

		case TWI_STOP:			// STOP condition or repeated START condition has been received while still addressed as Slave
			TWI0_SCTRLB = TWI_SCMD_COMPTRANS_gc;	// Prepare for next event. Should be new message.
// mod_led_toggle(6);
			break;
			
		case TWI_BUS_ERROR:		// Bus error
			TWI0_SCTRLB = TWI_SCMD_COMPTRANS_gc;	// Prepare for next event. Should be new message.
// mod_led_toggle(7);
			break;

		default:					// OOPS
			TWI0_SCTRLB = TWI_SCMD_COMPTRANS_gc;	// Prepare for next event. Should be new message.
// mod_led_toggle(8);
			break;
	}
}
