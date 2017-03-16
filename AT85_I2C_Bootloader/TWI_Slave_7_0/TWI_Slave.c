/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * TWI_Slave.c
 *
 * Created: 8/2011
 *  Author: AV112 Atmel
 * revision:	1/19/2017	v0.02	ndp	mod for USI I2C ATiny85 Bootloader
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "Common_Define.h"

#include "mod_led.h"
#include "spmUtils.h"

#include "USI_TWI_Slave.c"
#include "USI_TWI_Slave.h"

void InitTWI (void);
uint8_t IsBootPinLow (void);
uint8_t GetStatusCode (void);
void AbortTWI (void);
void ProcessSlaveTransmit (uint8_t data);
uint8_t SlaveReceiveByteAndACK (uint8_t * data);
uint8_t SlaveReceiveByteAndNACK (uint8_t * data);
void Erase_One_Page (uint16_t addr);
void UpdatePage (uint16_t pageAddress);

void resetBootReset();

void ProcessPageUpdate (void);
void Disable_WatchDogTimer (void);
void CleanupAndRunApplication (void);
void ProcessPageErase (void);
void Init_WatchDogTimer (void);
void ProcessSlaveReceive (void);
void Read_EEPROM_byte (uint8_t addr, unsigned char *cValue);
void ReadAndProcessPacket (void);
void Start_Timer (void);
void Host_Boot_Delay ();
int main(void);

void delay_cycles(uint16_t delay);		// added to replace __delay_cycles()

//configuring LAST_INTVECT_ADDRESS as per device selected
/*****************************************************************************/
#if defined(__AVR_ATtiny167__)
#define LAST_INTVECT_ADDRESS 	USI_START_vect// The start of the application code
#endif

#if defined(__AVR_ATtiny85__)
#define LAST_INTVECT_ADDRESS 	USI_OVF_vect_num	// The start of the application code
#endif

/*****************************************************************************/
uint8_t state = 0;

uint8_t pageBuffer[PAGE_SIZE] __attribute__ ((section (".noinit")));

uint16_t time_lapse_sec = 0;
uint8_t volatile gtimeout;
uint8_t statusCode = 0;

/***********************************************************************/
//Return non-zero if "Enter Bootloader" pin is held low externally.
// TODO Remove this.
uint8_t IsBootPinLow (void)
{
	// Make sure "Enter Bootloader" pin is input with internal pull-up.
	return 0;
}
/***********************************************************************/



/***********************************************************************/
uint8_t GetStatusCode (void)
{
	// Check if SPM operation is complete
	if ((SPMCSR & (1 << SELFPROGEN)) != 0)
	statusCode |= STATUSMASK_SPMBUSY;
	
	return statusCode;
}
/***********************************************************************/


#if 0
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
#endif

/***********************************************************************/

/***********************************************************************/

void ProcessPageUpdate (void)
{
	// Check the SPM is ready, abort if not.
	if ((SPMCSR & (1 << SELFPROGEN)) != 0)
	{
		USI_TWI_SLAVE_Abort ();
	}
}


/***********************************************************************/

/***********************************************************************/
void Disable_WatchDogTimer (void)
{
}

/***********************************************************************/

/***********************************************************************/

void CleanupAndRunApplication (void)
{
	Disable_WatchDogTimer(); // After Reset the WDT state does not change
	void (*FuncPtr) (void) = (void (*)(void)) ((LAST_INTVECT_ADDRESS + 2) / 2);	// Set up function pointer to address after last interrupt vector.
	FuncPtr ();
}

/***********************************************************************/

/***********************************************************************/

void ProcessPageErase (void)
{
	uint16_t addr = 0;
	uint8_t i;

	for (i = 0; i < PAGE_SIZE; ++i)
	{
		pageBuffer[i] = 0xFF;
	}

	UpdatePage (addr);		// To restore reset vector
	addr += PAGE_SIZE;

	for (i = 0; i < (LAST_PAGE_NO_TO_BE_ERASED - 1); i++, addr += PAGE_SIZE)
	{
		addr &= ~(PAGE_SIZE - 1);
		
		if (addr < BOOT_PAGE_ADDRESS)
		Erase_One_Page (addr); // Erase each page one by one until the bootloader section
	}
}

/***********************************************************************/

/***********************************************************************/

void Init_WatchDogTimer (void)
{
}
/*
// LO = 4C, HI = DE
const uint16_t index_calc[16] = {
0, 52225, 55297, 5120, 61441, 15360, 10240, 58369,
40961, 27648, 30720 , 46081, 20480, 39937, 34817, 17408 };
#define MODULO_TWO_SUM( arg1, arg2, arg3)   ( arg1 ^ arg2 ^ arg3 )
void ProcessCRCCheck(void)
{
//
//CRC_LO <<= ((1 << 2) -1);
//CRC_HI += 0x5a;
//
uint16_t crc_sum=0;
uint16_t j=0;
unsigned char const __flash * i = ( unsigned char const __flash * ) 2; // Leave out the reset vector for CRC
uint8_t data;
while (i < (unsigned char const __flash *) BOOT_PAGE_ADDRESS )
{
data = __load_program_memory(i);
j=index_calc[crc_sum & 0xF];
crc_sum = (crc_sum >> 4) & 0x0FFF;
crc_sum = MODULO_TWO_SUM(crc_sum, j, index_calc[data & 0xF]);

j= index_calc[crc_sum & 0xF];
crc_sum = (crc_sum >> 4) & 0x0FFF;
crc_sum = MODULO_TWO_SUM( crc_sum, j, index_calc[(data >> 4) & 0xF]);

i++;
}

CRC_HI = 0x00FF & (crc_sum >> 8);
CRC_LO = crc_sum & 0x00FF;
}
*/


/***********************************************************************/


/***********************************************************************/
void Read_EEPROM_byte (uint8_t addr, unsigned char *cValue)
{
	//  while (EECR & (1 << EEWE));	// Wait until EEWE becomes zero
	while (EECR & (1 << EEPE));	// Wait until EEWE becomes zero
	EEAR = addr;			// load eeprom adress
	EECR = (1 << EERE);		// read enable
	*cValue = EEDR;		// return eeprom data
}

/***********************************************************************/


/***********************************************************************/
void Start_Timer (void)
{
	TIFR   = TIFR;  // Clear flags //
	// TCNT1H  = 0;
	TCNT1  = 0;
	//OCR1A   = 7813;   // 7812.5 (one sec at 8 MHz clock operation)//
	OCR1A   = 255;   // 7812.5 (one sec at 8 MHz clock operation) //
	TCCR0A  = (1<<WGM01);	// mode4: CTC
	TCCR0B  = (1<<CS02) + (1<<CS00);	// prescaller: 1024
}

#define HOST_BOOT_DELAY_SEC 500

/***********************************************************************/

/***********************************************************************/
void Host_Boot_Delay ()
{
	//  time_lapse_sec = 0;
	Start_Timer ();
	
	// check this one later
	do{
		if (TIFR & _BV (OCF0A))
		{
			time_lapse_sec++;
			TIFR = TIFR;
			
		}
		
	}
	while (time_lapse_sec < HOST_BOOT_DELAY_SEC);
	TCCR0B = 0;
	TCCR0A =0;
	// stop the timer NOW
}

/***********************************************************************/

/***********************************************************************/
// Main Starts from here
int mainX (void)
{
	resetBootReset();

	mod_led_init();

	mod_led_toggle(2);
	
	if (MCUSR & (1<<PORF))	// Only in case of Power On Reset
	{
		MCUSR = 0;

		mod_led_on();
		Host_Boot_Delay ();
		mod_led_off();
	}
	
	if (!IsBootPinLow ())
	{
		USI_TWI_SLAVE_Init( SLAVE_ADDRESS );
		
		gtimeout = WDT_TIMEOUT_8s;
		Init_WatchDogTimer ();
		
		while (1)
		{
			// Process the USI based TWI Commands
			USI_TWI_SLAVE_ReadAndProcessPacket ();
mod_led_toggle(4);
		}
	}
	else
	{
		CleanupAndRunApplication ();
	}
	return 0;
}

// Added functions to repalce __delay_cycles()
void delay_cycles(uint16_t delay)
{
	for(uint16_t i=0; i<delay; ++i) {
		asm("nop");
	}
}
