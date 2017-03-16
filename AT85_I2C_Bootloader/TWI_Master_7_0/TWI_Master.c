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
 * mod_led.c
 *
 * Created: 8/2011			v0.01
 *  Author: 	AVR112 Atmel
 * revision:	1/21/2017	v0.02	ndp	mod for I2C ATmega164 and Atmel Studio 7.0
*/

#include <avr/io.h>
#include <stdbool.h>

#include "commands.h"

#include "serial.h"
#include "mod_led.h"

uint8_t MasterTransmit( uint8_t address, uint8_t * data, uint16_t length );
void delay_cycles(uint16_t delay);		// added to replace __delay_cycles()

#define _ATTINY85 1

// ATtiny85 signature bytes
// TODO Add command to read from Slave instead of here.
#define	SIGNATURE_BYTE_1	0x1E
#define	SIGNATURE_BYTE_2	0x93
#define	SIGNATURE_BYTE_3	0x0B

#define TWI_DDR_REG  DDRC
#define TWI_PORT_REG PORTC
#define TWI_SCL_PIN  PC0
#define TWI_SDA_PIN  PC1

#define CTS_DDR_REG      DDRD
#define CTS_PORT_REG     PORTD
#define CTS_PIN          PD3

#define RTS_DDR_REG      DDRD
#define RTS_PORT_REG     PORTD
#define RTS_PIN          PD2


// Page size selection for the controller with 2K flash
#if defined(_ATTINY25)   || defined(_ATTINY25V) || \
defined(_ATTINY24)   || defined(_ATTINY24A) || \
defined(_ATTINY2313) || defined(_ATTINY2313A) || \
defined(_ATTINY261)  || defined(_ATTINY261A)

#define PAGE_SIZE 32      //16 words = 32 Bytes
#define MAX__APP_ADDR 0X0400      // Maximum Application Address
#endif

// Page size selection for the controller with 4K flash
#if defined(_ATTINY45)   || defined(_ATTINY45V) || \
defined(_ATTINY44)   || defined(_ATTINY44A) || \
defined(_ATTINY4313) || defined(_ATTINY4313A) || \
defined(_ATTINY461)  || defined(_ATTINY461A) || \
defined(_ATTINY43U)  || \
defined(_ATTINY48)   || \
defined(_ATMEGA48)   || defined(_ATMEGA48A)  || \
defined(_ATMEGA48PA)

#define PAGE_SIZE 64      // 32 words = 64 Bytes
#define MAX__APP_ADDR 0X0C00      // Maximum Application Address
#endif

// Page size selection for the controller with 8K flash
#if defined(_ATTINY85)   || defined(_ATTINY85V) || \
defined(_ATTINY84)   || defined(_ATTINY84A) || \
defined(_ATTINY861)  || defined(_ATTINY861A) || \
defined(_ATTINY87)   || \
defined(_ATTINY88)

#define PAGE_SIZE 64      // 32 words = 64 Bytes
#define MAX__APP_ADDR 0X1800      // Maximum Application Address
// 0x1c00 means.. 1K bytes for Bootloader
// 0x1800 Means... 2K bytes for Bootloader
#endif

// Page size selection for the controller with 16K flash
#if defined(_ATTINY167)

#define PAGE_SIZE 128    // 64 words = 128 Bytes
#define MAX__APP_ADDR 0X3C00     // Maximum Application Address
#endif

#define SLAVE_ADDRESS 0xb0

unsigned char BlockLoad(unsigned int size, unsigned char mem);
/* BLOCKSIZE should be chosen so that the following holds: BLOCKSIZE*n = PAGESIZE,  where n=1,2,3... */

#define BLOCKSIZE PAGE_SIZE

#define TWI_CMD_PAGEUPDATE        0x01
#define TWI_CMD_EXECUTEAPP        0x02
#define TWI_CMD_AVERSION          0x03
#define TWI_CMD_BVERSION          0x04
#define TWI_CMD_ERASEFLASH        0x05
#define TWI_CMD_CRCCHECK          0x06
#define TWI_CMD_GETERRCONDN       0x07

#define TWI_START_TRANSMITTED     0x08
#define TWI_SLAW_ACKED            0x18
#define TWI_TXDATA_ACKED          0x28
#define TWI_TXDATA_NACKED         0x30
#define TWI_SLAR_ACKED            0x40
#define TWI_RXDATA_ACKED          0x50
#define TWI_RXDATA_NACKED         0x58

uint8_t statusCode;
uint8_t success;				// 0: No..1: Yes
uint8_t over_size_flag=0;

uint8_t pageBuffer[PAGE_SIZE+3];
uint16_t addr=0;

uint8_t command_char=0, data_char=0, CRC_HI=0, CRC_LO=0;
uint8_t runApp[2];
uint8_t reps=0;

uint8_t slaveCmdBuff[4];		// Use to send commands to Slave.


void InitTWI( void )
{
	TWI_DDR_REG &= ~((1 << TWI_SCL_PIN) | (1 << TWI_SDA_PIN));
	TWI_PORT_REG &= ~((1 << TWI_SCL_PIN) | (1 << TWI_SDA_PIN));
	
	// Init TWI as master.
	//	TWBR = 16; // 250bps @ 8MHz.
	TWBR = 92;						// 100k @ 20MHz.
	TWSR = 0;						// pre-scale TWPS1:0 = 00

	TWCR = (1 << TWEN);
}

// TODO
// Returns success.
uint8_t MasterReceive( uint8_t address, uint8_t * data, uint16_t length )
{
	uint8_t error = 0;

	// START condition.
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	do {} while ((TWCR & (1 << TWINT)) == 0);
	error = (TWSR != TWI_START_TRANSMITTED);

	// Send SLA+R.
	if (!error) {
		TWDR = (address & ~0x01) | 0x01; // LSB set = Read.
		TWCR = (1 << TWINT) | (1 << TWEN);
		do {} while ((TWCR & (1 << TWINT)) == 0);
		error = (TWSR != TWI_SLAR_ACKED);
	}
	
	// Read data bytes minus the last one. Return ACK.
	uint8_t * bufferPtr = data;
	if (!error) {
		for (uint16_t i = 0; i < (length - 1); ++i) {
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
			do {} while ((TWCR & (1 << TWINT)) == 0);
			error = (TWSR != TWI_RXDATA_ACKED);
			if (!error) {
				*bufferPtr = TWDR;
				++bufferPtr;
				} else {
				break;
			}
		}
	}
	
	// Read last data byte. Return NACK.
	if (!error) {
		TWCR = (1 << TWINT) | (1 << TWEN);
		do {} while ((TWCR & (1 << TWINT)) == 0);
		error = (TWSR != TWI_RXDATA_NACKED);
		if (!error) {
			*bufferPtr = TWDR;
			TWCR = (1 << TWSTO) | (1 << TWINT) | (1 << TWEN);
		}
	}

	// Abort communication if error.
	if (error) {
		TWCR = (1 << TWSTO) | (1 << TWINT) | (1 << TWEN);
	}
	
	return (!error);
}


// TODO
void get_slave_status(void)
{
	statusCode = 0;
	success = 1;
//	return;

	slaveCmdBuff[0] = CMD_GET_STATUS;
	(void) MasterTransmit( SLAVE_ADDRESS, slaveCmdBuff, 1 );

	do {
		success = MasterReceive( SLAVE_ADDRESS, &statusCode, 1 );
	} while ((statusCode != 0) || (!success));
}


// Returns success.
uint8_t MasterTransmit( uint8_t address, uint8_t * data, uint16_t length )
{
	uint8_t error = 0;
	
	// START condition.
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	do {} while ((TWCR & (1 << TWINT)) == 0);
	error = (TWSR != TWI_START_TRANSMITTED);

	// Send SLA+W.
	if (!error) {
		TWDR = (address & ~0x01) | 0x00; // LSB cleared = Write.
		TWCR = (1 << TWINT) | (1 << TWEN);
		do {} while ((TWCR & (1 << TWINT)) == 0);
		error = (TWSR != TWI_SLAW_ACKED);
	}
	
	// Send data bytes minus the last one. Expect ACK.
	const uint8_t * bufferPtr = data;
	if (!error) {
		for (uint16_t i = 0; i < (length - 1); ++i) {
			TWDR = *bufferPtr;
			TWCR = (1 << TWINT) | (1 << TWEN);
			do {} while ((TWCR & (1 << TWINT)) == 0);
			error = (TWSR != TWI_TXDATA_ACKED);
			if (!error) {
				++bufferPtr;
				} else {
				break;
			}
		}
	}
	
	// Send last data byte. Expect NACK.??? Not from Slave.
	if (!error) {
		TWDR = *bufferPtr;
		TWCR = (1 << TWINT) | (1 << TWEN);
		do {} while ((TWCR & (1 << TWINT)) == 0);
//		error = (TWSR != TWI_TXDATA_NACKED);					// BUG??
		error = (TWSR != TWI_TXDATA_ACKED);
		if (!error) {
			TWCR = (1 << TWSTO) | (1 << TWINT) | (1 << TWEN);
		}
	}

	// Abort communication if error.
	if (error) {
		TWCR = (1 << TWSTO) | (1 << TWINT) | (1 << TWEN);
	}
	
	return (!error);
}


#if 0
void First_Time(void)
{
	addr=0;
	pageBuffer[0] = TWI_CMD_PAGEUPDATE;
	get_slave_status();
}
#endif

// TODO
void read_from_slave(void)
{
	statusCode = 0;
	success = 1;
//	return;


	if (success)
	{
		do {
			success = MasterReceive( SLAVE_ADDRESS, &statusCode, 1 );
		} while ( !success );
	}
}


// TODO
void read_and_send(uint8_t whichversion)
{
	statusCode = 0;
	success = 1;

	runApp[0]=whichversion;
	runApp[1]=whichversion;
	get_slave_status();
	success = MasterTransmit( SLAVE_ADDRESS, runApp, 2 );
	read_from_slave();
	sendchar( statusCode );
}


// TODO
void send_command(uint8_t command)
{
	statusCode = 0;
	success = 1;
//	return;


	runApp[0] = command;
	runApp[1] = command;
	get_slave_status();
	success = MasterTransmit( SLAVE_ADDRESS, runApp, 2 );
}


/*************************************************************************/
//__C_task void main(void)
int main(void){
	unsigned int temp_int;
	unsigned char val = 0;

	CTS_DDR_REG |= (1<<CTS_PIN);	// set to output
	CTS_PORT_REG |= (1<<CTS_PIN);	// set high

	RTS_DDR_REG &= ~(1<<CTS_PIN);	// set to input
	RTS_PORT_REG |= (1<<CTS_PIN);	// enable pull-up
	
	mod_led_init();

mod_led_on();

	InitTWI();
	
	initbootuart(); // Initialize UART.

mod_led_off();

	/* Main loop */
	while(true)
	{
		val = recchar(); // Wait for command character.

		switch(val) {
			case 'P':
			case 'L':
			case 'E':
				sendchar('\r');
				break;
		
			// Read lock byte -> execute command
			case 'r':
				switch(command_char) {
					case 'a':
						read_and_send( TWI_CMD_AVERSION );
						break;

					case 'b':
						read_and_send( TWI_CMD_BVERSION );
						break;

					case 'd':
						// Read CRCHI
						sendchar(CRC_HI);
						break;

					case 'e':
						// Read CRCLO
						sendchar(CRC_LO);
						break;

					case 'f':
						// Status condition
						read_and_send(TWI_CMD_GETERRCONDN);
						break;

					default:
						sendchar(0xFF);
						break;
				}
				break;

			case 'l':
				// Write lock byte -> load command
				command_char = recchar();
				if( command_char == 'c' )
				{
					send_command( TWI_CMD_CRCCHECK );
					read_from_slave();
					CRC_HI= statusCode;
					read_from_slave();
					CRC_LO = statusCode;
				}
				sendchar('\r');
				break;

			case 'N':
				// Read high fuse bits -> BVERSION
				read_and_send( TWI_CMD_BVERSION );
				break;

			case 'F':
				// Low Fuse Bits -> AVERSION
				read_and_send( TWI_CMD_AVERSION );
				break;

			case	'a':
				sendchar('Y'); // Yes, we do auto-increment.
				break;

			case 'A':
 mod_led_toggle(2);
				addr =(recchar()<<8) | recchar(); // Read address high and low byte.
				if(addr > MAX__APP_ADDR) over_size_flag = 1;
				//+ 15mar17 ndp - send address to Slave.
				slaveCmdBuff[0] = CMD_RECV_ADRS;
				slaveCmdBuff[1] = (uint8_t)((addr>>8) & 0x00FF);				// AH
				slaveCmdBuff[2] = (uint8_t)(addr & 0x00FF);						// AL
				(void) MasterTransmit( SLAVE_ADDRESS, slaveCmdBuff, 3 );
				//-
				sendchar('\r'); // Send OK back.
 mod_led_toggle(2);
				break;

			case 'e':
				// Chip erase.
				runApp[0] =  TWI_CMD_ERASEFLASH;
				runApp[1] =  TWI_CMD_ERASEFLASH;
				get_slave_status();
				success = MasterTransmit( SLAVE_ADDRESS, runApp, 2 );
				sendchar('\r'); // Send OK back.
				break;
		
			case 'b':
				// Check block load support.
				sendchar('Y'); // Report block load supported.
				sendchar((BLOCKSIZE>>8) & 0xFF); // MSB first.
				sendchar(BLOCKSIZE&0xFF); // Report BLOCKSIZE (bytes).
				over_size_flag = 0;		// ndp 1-29-2017 fix
				break;
		
			case 'B':
				// Start block load.
				temp_int = (recchar()<<8) | recchar();	// Get block size.
				val = recchar();						// Get memtype.
				sendchar( BlockLoad(temp_int, val) );	// Block load.
//				if(reps == 0) First_Time();
 mod_led_toggle(4);			// Need a short delay here.
			 	pageBuffer[0] = CMD_RECV_DATA;					// Address was sent in 'A' command service.
				pageBuffer[1] = (uint8_t)(temp_int & 0x00FF);	// NL..Only block size less than 256 supported.
				// NOTE: Always sends PAGE_SIZE even if less data received from Host.
				success = MasterTransmit( SLAVE_ADDRESS, pageBuffer, pageBuffer[1]+2 );

//				if (success)
//				{
//					get_slave_status();
//				}
//				reps ++;
				break;
		
			case 'S':
				// Return programmer identifier.
				sendchar('A'); // Return 'AVRBOOT'.
				sendchar('V'); // Software identifier (aka programmer signature) is always 7 characters.
				sendchar('R');
				sendchar('B');
				sendchar('O');
				sendchar('O');
				sendchar('T');
				reps =0;
				break;
		
			case 'V':
				// Return software version.
				send_command(TWI_CMD_EXECUTEAPP);
				// Disable bootloader mode for slave
				sendchar('2');
				sendchar('0');
				break;

			case 's':
				// Return signature bytes [for the Target device ATtiny85].
#if 0
				sendchar( SIGNATURE_BYTE_3 );
				sendchar( SIGNATURE_BYTE_2 );
				sendchar( SIGNATURE_BYTE_1 );
#else
				slaveCmdBuff[0] = CMD_GET_SIG;
				(void) MasterTransmit( SLAVE_ADDRESS, slaveCmdBuff, 1 );
 mod_led_toggle(200);			// Need a short delay here to let Slave set up data.
				(void) MasterReceive( SLAVE_ADDRESS, slaveCmdBuff, 3 );
				sendchar( slaveCmdBuff[2] );
				sendchar( slaveCmdBuff[1] );
				sendchar( slaveCmdBuff[0] );
#endif
				break;
		
			/* Add missing command .. ndp 01-29-2017
			 * Return Flash Data.
			 *
			 * TODO: Need to read from Slave.
			 */
			case 'g':
 mod_led_toggle(4);
 				temp_int = (recchar()<<8) | recchar();	// Get block size.
				val = recchar();						// Get mem type.
 #if 0
				for(int i=0; i<temp_int; ++i)
				{
					sendchar( 0 );
				}
#else
				// NOTE: Address was sent in 'A' command process.
				slaveCmdBuff[0] = CMD_GET_DATA;
				slaveCmdBuff[1] = (uint8_t)(temp_int & 0x00FF);
				(void) MasterTransmit( SLAVE_ADDRESS, slaveCmdBuff, 2 );
 mod_led_toggle(200);			// Need a short delay here to let Slave set up data.
				(void) MasterReceive( SLAVE_ADDRESS, pageBuffer, (temp_int & 0x00FF) );

				for(int i=0; i<temp_int; ++i)
				{
					sendchar( pageBuffer[i] );
				}
#endif
 mod_led_toggle(4);
				break;

			default:
				if(val != 0x1b) {                  // If not ESC, then it is unrecognized...
					sendchar('?');
				}
				break;
		} // end: switch()
	} // end: while(true)
} // end: main

unsigned char BlockLoad(unsigned int size, unsigned char mem)
{
	if(!over_size_flag) // Check for file size to be less than maximum pages that can be programmed
	{
		if(mem == 'F')
		{
			// Flash memory type.
			for (uint16_t i = 0; i < size; ++i)
			{
				pageBuffer[i+2] = recchar();
			}
			return '\r'; // Report programming OK
		}
		else
		{
			// Invalid memory type?
			return '?';
		}
	}
	else
	{
		over_size_flag=0;
		return '?';
	}
}

// Added functions to repalce __delay_cycles()
void delay_cycles(uint16_t delay)
{
	for(uint16_t i=0; i<delay; ++i) {
		asm("nop");
	}
}
