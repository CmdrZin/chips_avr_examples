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
 * usiTwiSlaveNI.c
 *
 * Created: 1/24/2017		0.01	ndp
 *  Author: Chip
 */ 
#include <avr/io.h>
#include <stdbool.h>

#include "usiTwiSlaveNI.h"

#include "mod_led.h"			// FOR TEST ONLY


void putRxFifo(uint8_t data);
uint8_t getRxFifo();
void putTxFifo(uint8_t data);
uint8_t getTxFifo();

 // I/O defines for ATtiny85
#define USI_DDR		DDRB
#define USI_PORT	PORTB
#define USI_PIN		PINB
#define USI_SDA_PIN	PB0
#define USI_SCL_PIN	PB2

#define UTS_FIFO_SIZE	68
uint8_t utsTxFifo[UTS_FIFO_SIZE];
uint8_t utsTxHead;
uint8_t utsTxTail;
uint8_t utsRxFifo[UTS_FIFO_SIZE];
uint8_t utsRxHead;
uint8_t utsRxTail;


UTS_STATE utsState;
uint8_t	utsSlaveAdrs;
bool utsSend;

/*
 * adrs = Slave address (7bit)
 */
void uts_init(uint8_t adrs)
{
	utsState = UTS_WAIT_FOR_START;
	utsSlaveAdrs = adrs;
	utsSend = true;						// default to SDA_R send data to Master
	utsTxHead = utsTxTail = utsRxHead = utsRxTail = 0;

	USI_DDR &= ~(1<<USI_SDA_PIN);		// SDA as input
	USI_PORT |= (1<<USI_SDA_PIN);		// enable pull-up
	USI_DDR |= (1<<USI_SCL_PIN);		// SCL as output
	USI_PORT |= (1<<USI_SCL_PIN);

	// Clear interrupt flags by writing 1 to them and clear counter to 0.
	USISR = (1<<USISIF) |				// Clear START flag
			(1<<USIOIF) |				// Clear Counter OV flag
			(1<<USIPF); 				// Clear STOP flag
					                    // Clear USICNT<3:0> counter to 0.

	// Enable START detect and configure USI
	USICR = (1<<USISIE) |				// Enable START detect.
			(0<<USIOIE) |				// Disable Counter OV detect.
			(1<<USIWM1) | (0<<USIWM0) |	// Set USI in Two-wire mode w/o clock stretch.
			(1<<USICS1) | (0<<USICS0) | (0<<USICLK);	// Clock Source = External, positive, both edges
}


 /*
  * Call this as often as possible to check register flags that would normally
  * be serviced by an interrupt handler.
  * Registers of Interest:
  *   USICR		
  *   USISR		
  */
void uts_poll()
{
	if(USISR & (1<<USISIF)) {
		// SDA(0) while SCL(1) is START. Need to wait for SCL(0) before counting clocks.
		while(USI_PIN & (1<<USI_SCL_PIN));			// Wait for SCL(0)
		// Set SDA for input
		USI_DDR &= ~(1<<USI_SDA_PIN);
		// Enable Counter OV detect.
		USICR |= (1<<USIOIE);
		// Set up counter to bring in address.
		// Clear Counter OV and reset counter.
		USISR = (1<<USIOIF);
		USISR |= (1<<USISIF);		// clear START detect bit also to release SCL
		utsState = UTS_WAIT_FOR_ADRS;

		USICR |= (1<<USIWM0);		// Enable OV clock stretch
	}

	if(USISR & (1<<USIOIF)) {
		// OV is cleared after case: service. SCL held off till then.

		switch(utsState) {
			case UTS_WAIT_FOR_ADRS:
				// get adrs, check it, and return ACK/NAK as needed.
				if( (USIDR>>1) == utsSlaveAdrs) {
					// Get Read/Write bit.
					utsSend = ((USIDR & 0x01) == 1);
					// Send ACK
					uts_sendACK();
					utsState = UTS_WAIT_FOR_ADRS_ACK;
				} else {
					// For NAK, just don't respond. Line will stay high.
					// Go back to waiting for START.
					utsState = UTS_WAIT_FOR_START;
				}
				USISR |= (1<<USIOIF);				// clear OV detect bit
				break;

			case UTS_WAIT_FOR_ADRS_ACK:
				// Receiving Address.
				// Check for Counter OV
				// Set SDA as input to clear ACK.
				USI_DDR &= ~(1<<USI_SDA_PIN);
				// Data follows sending Address ACK.
				if(utsSend) {
					// Load up data register for output.
					USIDR = getTxFifo();
					// Set SDA for output
					USI_DDR |= (1<<USI_SDA_PIN);
					// Reset counter.
					USISR = 0;
					utsState = UTS_WAIT_FOR_DATA_OUT;
				} else {
					// Set SDA for input
					USI_DDR &= ~(1<<USI_SDA_PIN);
					// Reset counter.
					USISR = 0;
					utsState = UTS_WAIT_FOR_DATA_IN;
				}
				USISR |= (1<<USIOIF);				// clear OV detect bit
				break;

			case UTS_WAIT_ON_ACK:
				// Receiving Data.
				// Set SDA as input to clear ACK.
				USI_DDR &= ~(1<<USI_SDA_PIN);
				// Reset counter.
				USISR = 0;
				utsState = UTS_WAIT_FOR_DATA_IN;
	
				USISR |= (1<<USIOIF);				// clear OV detect bit
				break;

			case UTS_WAIT_FOR_DATA_IN:
				// Check for Counter OV
				// Get data..put into fifo.
				putRxFifo(USIDR);
				// Send ACK
				uts_sendACK();
				utsState = UTS_WAIT_ON_ACK;
				USISR |= (1<<USIOIF);				// clear OV detect bit
				break;

			case UTS_WAIT_FOR_DATA_OUT:
				// Data sent.
				uts_chkACK();
				// Look for ACK/NAK
				utsState = UTS_WAIT_CHK_ACK;
				USISR |= (1<<USIOIF);				// clear OV detect bit
				break;

			case UTS_WAIT_CHK_ACK:
				// Sending Data
				// Check for Counter OV
				// Check Data reg for ACK(0) or NAK(1)
				if((USIDR & 0x01) == 0) {
					// Load up data register for more output.
					USIDR = getTxFifo();
					// Set SDA for output
					USI_DDR |= (1<<USI_SDA_PIN);
					// Clear Counter OV and reset counter.
					USISR = 0;
					utsState = UTS_WAIT_FOR_DATA_OUT;
				} else {
					// Master doesn't want any more.
					// Go back to waiting for another message.
					utsState = UTS_WAIT_FOR_START;

					USICR &= ~(1<<USIWM0);		// Disable OV clock stretch
				}
				USISR |= (1<<USIOIF);				// clear OV detect bit
				break;

			default:
				uts_init(utsSlaveAdrs);					// restart interface
				break;
		}  // end switch()
	} // end if(OV)

	// Check for STOP.
	if(USISR & (1<<USIPF)) {
		USICR &= ~(1<<USIOIE);				// Disable Counter OV detect.
		// Clear interrupt flags by writing 1 to them and clear counter to 0.
		USISR = (1<<USISIF) |				// Clear START flag
			(1<<USIOIF) |				// Clear Counter OV flag
			(1<<USIPF); 				// Clear STOP flag
		utsState = UTS_WAIT_FOR_START;
		USICR &= ~(0<<USIWM0);		// Disable OV clock stretch
	}
}

/*
 * Set up counter to send 1 bit as ACK(0).
 * For NAK, just don't do anything.
 */
void uts_sendACK()
{
	// Set ACK(0) bit
	USIDR = 0;
	// set SDA as output
	USI_DDR |= (1<<USI_SDA_PIN);
	// Set counter to output 1 bit.
	USISR = 0x0E;
}


/*
 * Set up to read in the ACK/NAK bit on the 9th clock.
 */
void uts_chkACK()
{
	// Set SDA as input
	USI_DDR &= ~(1<<USI_SDA_PIN);
	// Clear data register
	USIDR = 0;
	// Set counter to input 1 bit.
	USISR = 0x0E;
}

void clearRxFifo()
{
	utsRxHead = utsRxTail = 0;
}

void clearTxFifo()
{
	utsTxHead = utsTxTail = 0;
}

bool isRxEmpty()
{
	return (utsRxHead == utsRxTail);
}

bool isTxEmpty()
{
	return (utsTxHead == utsTxTail);
}

// Simple FIFOs. NO overflow checks.
uint8_t getRxFifo()
{
	uint8_t data;

	data = utsRxFifo[utsRxTail];
	if(++utsRxTail >= UTS_FIFO_SIZE) utsRxTail = 0;

	return data;
}

void putRxFifo(uint8_t data)
{
	utsRxFifo[utsRxHead] = data;
	if(++utsRxHead >= UTS_FIFO_SIZE) utsRxHead = 0;
}

void putTxFifo(uint8_t data)
{
	utsTxFifo[utsTxHead] = data;
	if(++utsTxHead >= UTS_FIFO_SIZE) utsTxHead = 0;
}

uint8_t getTxFifo()
{
	uint8_t data;

	data = utsTxFifo[utsTxTail];
	if(++utsTxTail >= UTS_FIFO_SIZE) utsTxTail = 0;

	return data;
}
