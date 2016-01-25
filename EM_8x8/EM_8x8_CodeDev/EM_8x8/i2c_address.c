/*
 * i2c_address.c
 *
 * Created: 1/17/2016 10:07:38 AM
 *  Author: Chip
 *
 * This module supports external jumpers to adjust the I2C address.
 */ 

#include <avr/io.h>

#include "i2c_address.h"

#define I2C_ADRS_DDR	DDRC
#define I2C_ADRS_PORT	PORTC
#define I2C_ADRS_PIN	PINC

#define I2C_ADRS_D0		PORTC0
#define I2C_ADRS_D1		PORTC1
#define I2C_ADRS_D2		PORTC2
#define I2C_ADRS_D3		PORTC3


/*
 * This function returns the I2C address modified by the jumpers.
 * Change as needed to match hardware.
 */
uint8_t ia_getAddress()
{
	uint8_t adrs;
	
	// Default is INPUT
	// Set Pull-Ups and wait for a bit to stabilize
	for(adrs = 0; adrs<20; ++adrs)
	{
		I2C_ADRS_PORT |= (1<<I2C_ADRS_D3);
		I2C_ADRS_PORT |= (1<<I2C_ADRS_D2);
		I2C_ADRS_PORT |= (1<<I2C_ADRS_D1);
		I2C_ADRS_PORT |= (1<<I2C_ADRS_D0);
	}

	// Inverted to make jumpers 1 if inserted and 0 if not.
	adrs = ~(I2C_ADRS_PIN);
	//  Mask to match pins used.
	adrs &= (1<<I2C_ADRS_D3)|(1<<I2C_ADRS_D2)|(1<<I2C_ADRS_D1)|(1<<I2C_ADRS_D0);
		
	return( SLAVE_ADRS + adrs );	// TODO: Pull this from EEPROM someday.
}
