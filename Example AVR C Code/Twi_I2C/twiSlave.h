/*
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