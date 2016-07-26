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
 * twiSlave.h
 *
 * Created: 8/10/2015	0.01	ndp
 *  Author: Chip
 *
 */ 


#ifndef TWISLAVE_H_
#define TWISLAVE_H_

#include <stdbool.h>


/* *** Buffer defines *** */
// allowed buffer sizes: 2^n up to 256 bytes

#define TWI_RX_BUFFER_SIZE  ( 32 )
#define TWI_RX_BUFFER_MASK  ( TWI_RX_BUFFER_SIZE - 1 )

#if ( TWI_RX_BUFFER_SIZE & TWI_RX_BUFFER_MASK )
#  error TWI_RX_BUFFER_SIZE is not a power of 2
#endif

#define TWI_TX_BUFFER_SIZE ( 32 )
#define TWI_TX_BUFFER_MASK ( TWI_TX_BUFFER_SIZE - 1 )

#if ( TWI_TX_BUFFER_SIZE & TWI_TX_BUFFER_MASK )
#  error TWI_TX_BUFFER_SIZE is not a power of 2
#endif


/* *** GLobal Protoptyes *** */

void	twiSlaveInit( uint8_t adrs );		// Set up TWI hardware and set Slave I2C Address.
void	twiSlaveEnable( void );				// Enable I2C Slave interface.

void	twiTransmitByte( uint8_t data );	// Place data into output buffer.
uint8_t	twiReceiveByte( void );				// Read data from input buffer.
bool	twiDataInReceiveBuffer( void );		// Check for available data in input buffer. This function should return TRUE
											// before calling twiReceiveByte() to get data.
void	twiClearOutput( void );				// Reset the output buffer to empty. Used recover from sync errors.

void	twiStuffRxBuf( uint8_t data );	// Allows manual input into input buffer for testing.


#endif /* TWISLAVE_H_ */