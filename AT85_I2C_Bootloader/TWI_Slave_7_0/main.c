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
 * TWI_Slave_7_0.c
 *
 * Created: 3/14/2017	0.01	ndp
 * Author : Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "Common_Define.h"
#include "commands.h"

#include "usiTwiSlaveNI.h"
#include "mod_led.h"
#include "spmUtils.h"

// ATtiny85
#define	SIGNATURE_BYTE_1	0x1E
#define	SIGNATURE_BYTE_2	0x93
#define	SIGNATURE_BYTE_3	0x0B


typedef enum cmdModes { CM_IDLE, CM_ADRS_HI, CM_ADRS_LO, CM_RECV_DATA, CM_RECV_SIZE, CM_SEND_DATA } CM_MODE;

void commandProcess(uint8_t cmd);
void initBootTimer(uint8_t waitTime);
bool waitBootTimeOut();
void resetBootReset();
void Erase_One_Page (uint16_t addr);
void UpdatePage (uint16_t pageAddress);

uint8_t	bootWaitTime;
uint8_t	systemStatus;
CM_MODE commandMode;
uint16_t workingAdrs;				// Flash address
uint8_t	dataCount;
uint8_t	pageBuffer[PAGE_SIZE];
uint8_t	dataIndex;
uint8_t	appResetBytes[2];


int main(void)
{
	mod_led_init();
	mod_led_on();

	systemStatus = 0;

	resetBootReset();				// overwrite App code reset vector

//	mod_led_toggle(2);

	uts_init(SLAVE_ADDRESS>>1);		// adj for standard use. (0xb0 -> 0x58)

	initBootTimer(5);

	putTxFifo(0x00);				// preload TxFifo to test for a Read.

	while( waitBootTimeOut() ) {
		if( !isRxEmpty() || isTxEmpty() ) {
			// Either a Write+data or a Read on I2C occurred. Activate Bootloader else check for app.
			break;
		}
	}

	mod_led_off();

    while (1) 
    {
		uts_poll();					// Service non-interrupt I2C handler.

		if(!isRxEmpty()) {
 mod_led_toggle(6);
			commandProcess(getRxFifo());
		}
    }
}


void commandProcess(uint8_t cmd)
{
	switch(commandMode) {
		case CM_IDLE:									// Waiting for NEW command sequence
			switch(cmd) {
				case CMD_GET_STATUS:					// Request status
 mod_led_toggle(1);
					putTxFifo(systemStatus);
					break;

				case CMD_RECV_ADRS:
 mod_led_toggle(2);
					commandMode = CM_ADRS_HI;
					break;

				case CMD_RECV_DATA:
 mod_led_toggle(3);
					commandMode = CM_RECV_SIZE;			// Expecting one page block of data (0x40)
					break;

				case CMD_GET_DATA:
 mod_led_toggle(4);
					commandMode = CM_SEND_DATA;
					clearTxFifo();
					break;

				case CMD_GET_SIG:
 mod_led_toggle(5);
					commandMode = CM_IDLE;
					clearTxFifo();
					putTxFifo(SIGNATURE_BYTE_1);
					putTxFifo(SIGNATURE_BYTE_2);
					putTxFifo(SIGNATURE_BYTE_3);
					clearRxFifo();
					break;

				default:
 mod_led_toggle(8);
					break;
			}
			break;

		case CM_ADRS_HI:
 mod_led_toggle(10);
			workingAdrs = cmd;
			commandMode = CM_ADRS_LO;
			break;

		case CM_ADRS_LO:
 mod_led_toggle(11);
			workingAdrs = (workingAdrs<<8) + cmd;
			workingAdrs <<= 1;							// Convert to bytes address.
			commandMode = CM_IDLE;
			break;

		case CM_RECV_SIZE:
 mod_led_toggle(12);
			dataCount = cmd;
			dataIndex = 0;
			commandMode = CM_RECV_DATA;
			break;

		case CM_RECV_DATA:								// Address already received and in workingAdrs.
 mod_led_toggle(13);
			pageBuffer[dataIndex] = cmd;
			if(++dataIndex == dataCount) {
				// Write dataBuffer to Flash
				UpdatePage(workingAdrs);
				commandMode = CM_IDLE;
			}
			break;

		case CM_SEND_DATA:
 mod_led_toggle(14);
			dataCount = cmd;
			commandMode = CM_IDLE;
 mod_led_on();
			if(workingAdrs == 0) {
				// Restore App Reset bytes if needed. Page 0.
				putTxFifo(appResetBytes[0]);
				putTxFifo(appResetBytes[1]);
				workingAdrs += 2;
				for(int i=2; i<dataCount; i++) {
					putTxFifo(pgm_read_byte(workingAdrs++));
				}
			} else {
				// Send non-0 page.
				for(int i=0; i<dataCount; i++) {
					putTxFifo(pgm_read_byte(workingAdrs++));
				}
			}
 mod_led_off();
			break;			

		default:
 mod_led_toggle(20);
			commandMode = CM_IDLE;
			break;
	}
}


/* ***************** UTILITIES ******************************************* */

/*
 * Use Timer1 to wait N seconds for an I2C action to enter Bootloader.
 * Assumes an 8 MHz clock
 */
void initBootTimer(uint8_t waitTime)
{
	TCCR1 = ((1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10));	// Use Tclk/16384 Prescaler (~2ms)
	
	bootWaitTime = waitTime << 1;			// N*2
}

/*
 * Check for timeout. Overflows every 256*2ms (~0.5s)
 */
bool waitBootTimeOut()
{
	if(TIFR & (1<<TOV1)) {
		TIFR |= (1<<TOV1);				// clear OV flag

		--bootWaitTime;
		if(bootWaitTime == 0) {
			TCCR1 = 0;					// disable Timer1
			return false;
		}
	}

	return true;
}

/*
 * Reload Reset Vector in Page 0 to jump to Bootloader.
 * Added: 1/29/2017 ndp
 */
void resetBootReset()
{
	uint16_t tempWord;
	uint16_t tempAddress = 0;

	// Load RESET vector to the Bootloader into buffer.
	tempWord = ((BOOT_PAGE_ADDRESS-1)>>1) | 0xC000;			// get address and add rjmp op code.
	spm_filltemp(tempAddress, tempWord);
	tempAddress += 2;

	// Read in FLASH Page 0
	for(int i=2; i<PAGE_SIZE; i += 2) {
		tempWord = pgm_read_word(i);
		spm_filltemp(tempAddress, tempWord);
		tempAddress += 2;
	}

	while(SPMCSR & (1<<SPMEN));		// wait

	// Write Page 0 from temporary buffer
	spm_pagewrite(0);
	do{} while ((SPMCSR & (1 << SELFPROGEN)) != 0); // Wait for the SPM operation to complete
}

/***********************************************************************/
/*  Absolute Unconditional Page Erase  Check bounds elsewhere    */
void Erase_One_Page (uint16_t addr)
{
	// Erase page at the given address
	spm_erase(addr);

	do{} while ((SPMCSR & (1 << SELFPROGEN)) != 0);
	
}
/***********************************************************************/

/***********************************************************************/
void UpdatePage (uint16_t pageAddress)
{
	// Mask out in-page address bits.
	pageAddress &= ~(PAGE_SIZE - 1);
	// Protect RESET vector if this is page 0.
	if (pageAddress == INTVECT_PAGE_ADDRESS)
	{
		// Save App Reset Vector
		appResetBytes[0] = pageBuffer[0];
		appResetBytes[1] = pageBuffer[1];
		// Load existing RESET vector contents into buffer.
		pageBuffer[0] = pgm_read_byte((void *)(INTVECT_PAGE_ADDRESS + 0));
		pageBuffer[1] = pgm_read_byte((void *)(INTVECT_PAGE_ADDRESS + 1));
	}

	// Ignore any attempt to update boot section.
	if (pageAddress < BOOT_PAGE_ADDRESS)
	{
		Erase_One_Page (pageAddress);

		// Load temporary page buffer.
		uint8_t *bufferPtr = pageBuffer;
		uint16_t tempAddress = pageAddress;
		for (uint8_t i = 0; i < PAGE_SIZE; i += 2)
		{
			uint16_t tempWord = ((bufferPtr[1] << 8) | bufferPtr[0]);
			// Fill the temporary buffer with the given data
			spm_filltemp(tempAddress, tempWord);

			tempAddress += 2;
			bufferPtr += 2;
		}
		// Write page from temporary buffer to the given location in flash memory
		spm_pagewrite(pageAddress);

		do{} while ((SPMCSR & (1 << SELFPROGEN)) != 0); // Wait for the SPM operation to complete
	}
}
