/*
 * debugAT85.c
 *
 * Created: 7/11/2015 11:40:34 PM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sysTimer.h"
// DEV_1MS_TIC, 

#include "usiTwiSlave.h"
#include "access.h"
#include "eeprom_util.h"

#include "debugAT85.h"

const char debugVersion[] PROGMEM = "1.00";		// add spaces to pad to 4 characters.
const char buildDate[] PROGMEM = __DATE__;		// 11 bytes
const char buildTime[] PROGMEM = __TIME__;		// 8 bytes


#define DB_PIN2_P		PB3
#define DB_PIN2_DDR		DDRB
#define DB_PIN2_PORT	PORTB
#define DB_PIN2_PIN		PINB

#define DB_PIN3_P		PB4
#define DB_PIN3_DDR		DDRB
#define DB_PIN3_PORT	PORTB
#define DB_PIN3_PIN		PINB

#define DB_PIN6_P		PB1
#define DB_PIN6_DDR		DDRB
#define DB_PIN6_PORT	PORTB
#define DB_PIN6_PIN		PINB


// Current output from ADC. Left justified.
uint8_t dev_debug_adc_1_H;
uint8_t dev_debug_adc_1_L;

// Parameters
DEBUG_VREF debug_adc_vref;

/*
 * Debug initialization.
 */
void dev_debug_init()
{
	uint16_t val;

	// Check for parameter. Set to default if not found.
	val = param_get_value(DB_ADC_VREF);
	if(val == PARAM_NO_KEY) {
		param_update(DB_ADC_VREF, DB_VREF_VCC);
		debug_adc_vref = DB_VREF_VCC;
	} else {
		debug_adc_vref = val;
	}
}

void dev_debug_service()
{
	// NO dynamic services at this time.
	return;
}


/* *** DEBUG ACCESS FUNCTIONS *** */

/*
 * Command 0x30 -  Read IO PIN
 * DEV 0x30 0x01 PIN. PIN: 2,3,6
 */
void dev_debug_read_pin()
{
	uint8_t pin = getCmdData(3);
	uint8_t data = 0;

	switch(pin) {
		case 2:
			DB_PIN2_DDR &= ~(1<<DB_PIN2_P);				// set as input
			data = DB_PIN2_PIN & (1<<DB_PIN2_P);		// read port and mask pin
			break;

		case 3:
			DB_PIN3_DDR &= ~(1<<DB_PIN3_P);				// set as input
			data = DB_PIN3_PIN & (1<<DB_PIN3_P);		// read port and mask pin
			break;

		case 6:
			DB_PIN6_DDR &= ~(1<<DB_PIN6_P);				// set as input
			data = DB_PIN6_PIN & (1<<DB_PIN6_P);		// read port and mask pin
			break;
	}

	// Convert to 0 or 1
	usiTwiTransmitByte( DEV_DEBUG_ID );
	(data == 0) ? (data = 0) : (data = 1);
	usiTwiTransmitByte( data );
}

/*
 * Command 0x40 -  Set IO PIN LOW
 * DEV 0x40 0x01 PIN. PIN: 2,3,6
 */
void dev_debug_set_pin_low()
{
	uint8_t pin = getCmdData(3);
	debug_set_pin_low(pin);
}

void debug_set_pin_low(uint8_t pin)
{
	switch(pin) {
		case 2:
			DB_PIN2_DDR |= (1<<DB_PIN2_P);				// set as output
			DB_PIN2_PORT &= ~(1<<DB_PIN2_P);			// set pin LOW
			break;

		case 3:
			DB_PIN3_DDR |= (1<<DB_PIN3_P);				// set as output
			DB_PIN3_PORT &= ~(1<<DB_PIN3_P);			// set pin LOW
			break;

		case 6:
			DB_PIN6_DDR |= (1<<DB_PIN6_P);				// set as output
			DB_PIN6_PORT &= ~(1<<DB_PIN6_P);			// set pin LOW
			break;
	}
}

/*
 * Command 0x41 -  Set IO PIN HIGH
 * DEV 0x41 0x01 PIN. PIN: 2,3,6
 */
void dev_debug_set_pin_high()
{
	uint8_t pin = getCmdData(3);
	debug_set_pin_high(pin);
}

void debug_set_pin_high(uint8_t pin)
{
	switch(pin) {
		case 2:
			DB_PIN2_DDR |= (1<<DB_PIN2_P);			// set as output
			DB_PIN2_PORT |= (1<<DB_PIN2_P);			// set pin HIGH
			break;

		case 3:
			DB_PIN3_DDR |= (1<<DB_PIN3_P);			// set as output
			DB_PIN3_PORT |= (1<<DB_PIN3_P);			// set pin HIGH
			break;

		case 6:
			DB_PIN6_DDR |= (1<<DB_PIN6_P);			// set as output
			DB_PIN6_PORT |= (1<<DB_PIN6_P);			// set pin HIGH
			break;
	}
}

void dev_pulse_pin(uint8_t pin, uint8_t time)
{
	uint8_t count = time;
	// Wait for 1ms TIC
	if( GPIOR0 & (1<<DEV_1MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_1MS_TIC);
		// call set pin HIGH
		debug_set_pin_high(pin);
		// Wait for N 1ms TICs
		while(count != 0)
		{
			if( GPIOR0 & (1<<DEV_1MS_TIC) )
			{
				GPIOR0 &= ~(1<<DEV_1MS_TIC);
				--count;
			}
		}
		// call set pin LOW
		debug_set_pin_low(pin);
	}
}

/*
 * Command 0xA0 -  Pulse PIN P for N ms
 * DEV 0xA0 0x02 PIN MS
 * PIN: 2,3,6
 */
void dev_debug_pulse_pin()
{
	uint8_t pin = getCmdData(3);
	dev_pulse_pin(pin, getCmdData(4));
}

/*
 * Trigger ADC channel
 *
 * input:	channel
 *
 * output:	dev_adc_1_L = ADCL
 * 			dev_adc_1_H = ADCH
 *
 * NOTE: MUST read ADCL first then ADCH.
 */
void dev_adc_trigger(uint8_t chan)
{
	// Set Vref and data format.
	switch(debug_adc_vref) {
		case DB_VREF_2_56:
			ADMUX = (1<<REFS2)|(1<<REFS1)|(0<<REFS0)|(1<<ADLAR)|(chan & 0x07);	// Internal Vref = 2.56v, Left Adj, lower 3 bits select channel.
			break;
		case DB_VREF_VCC:
		default:
			ADMUX = (0<<REFS2)|(0<<REFS1)|(0<<REFS0)|(1<<ADLAR)|(chan & 0x07);	// Internal Vref = Vcc, Left Adj, lower 3 bits select channel.
			break;
	}
	// Enable ADC, set clock rate
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);	// ADC Enable, CPU/64 clock (8MHz => 125kHz or ~120us/sample)

	// trigger ADC sample.
	ADCSRA |= (1<<ADSC);		// trigger
	// Wait ~120us
	while( (ADCSRA & (1<<ADSC)) )
	{
		asm("nop");
	}
	// Get Data
	dev_debug_adc_1_L = ADCL;		// get 2 LSBs .. d7:6
	dev_debug_adc_1_H = ADCH;		// get 8 MSBs .. d7:0..releases ADC data reg.

	// Get ADC Data and put it into output fifo with device ID
	usiTwiTransmitByte( DEV_DEBUG_ID );
	usiTwiTransmitByte( dev_debug_adc_1_H );
	usiTwiTransmitByte( dev_debug_adc_1_L );
}

/*
 * Command 0x70 -  Read ADC Channel
 * DEV 0x70 0x01 CHAN
 * CHAN: 2,3
 * Loads output FIFO with ADC channel data.
 *
 * NOTE: ADC0 and ADC1 are used by the I2C interface.
 */
void dev_debug_get_adc()
{
	uint8_t chan = getCmdData(3);
	dev_adc_trigger(chan);
}

/*
 * Command 0x78 - Set Vref to Vcc
 */
void dev_debug_set_vref_vcc()
{
	debug_adc_vref = DB_VREF_VCC;
}

/*
 * Command 0x7B - Set Vref to 2.56
 */
void dev_debug_set_vref_2_56()
{
	debug_adc_vref = DB_VREF_2_56;
}

/*
 * Return build DATE stamp.
 */
void dev_debug_get_build_date()
{
	char data;
	uint8_t index;
	
	// Get data and put it into output fifo with device ID
	index = 0;
	usiTwiTransmitByte( DEV_DEBUG_ID );
	while( (data = pgm_read_byte(&(buildDate[index]))) != 0 && (index < 15) )
	{
		usiTwiTransmitByte( data );
		++index;
	}
}

/*
 * Return build TIME stamp.
 */
void dev_debug_get_build_time()
{
	char data;
	uint8_t index;
	
	// Get data and put it into output fifo with device ID
	index = 0;
	usiTwiTransmitByte( DEV_DEBUG_ID );
	while( (data = pgm_read_byte(&(buildTime[index]))) != 0 && (index < 15) )
	{
		usiTwiTransmitByte( data );
		++index;
	}
}

/*
 * Return debug Version.
 */
void dev_debug_get_version()
{
	char data;
	uint8_t index;
	
	// Get data and put it into output fifo with device ID
	index = 0;
	usiTwiTransmitByte( DEV_DEBUG_ID );
	while( (data = pgm_read_byte(&(debugVersion[index]))) != 0 && (index < 15) )
	{
		usiTwiTransmitByte( data );
		++index;
	}
}
