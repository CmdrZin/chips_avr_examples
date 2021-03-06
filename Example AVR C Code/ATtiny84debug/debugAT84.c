/*
 * debugAT84.c
 *
 * Created: 6/8/2015 9:54:34 PM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sysTimer.h"
// DEV_1MS_TIC, 

#include "usiTwiSlave.h"
#include "access.h"
#include "eeprom_util.h"

#include "debugAT84.h"

const char debugVersion[] PROGMEM = "1.01";		// add spaces to pad to 4 characters.
const char buildDate[] PROGMEM = __DATE__;		// 11 bytes
const char buildTime[] PROGMEM = __TIME__;		// 8 bytes


#define DB_PIN13_P		PA0
#define DB_PIN13_DDR	DDRA
#define DB_PIN13_PORT	PORTA
#define DB_PIN13_PIN	PINA

#define DB_PIN12_P		PA1
#define DB_PIN12_DDR	DDRA
#define DB_PIN12_PORT	PORTA
#define DB_PIN12_PIN	PINA

#define DB_PIN11_P		PA2
#define DB_PIN11_DDR	DDRA
#define DB_PIN11_PORT	PORTA
#define DB_PIN11_PIN	PINA

#define DB_PIN10_P		PA3
#define DB_PIN10_DDR	DDRA
#define DB_PIN10_PORT	PORTA
#define DB_PIN10_PIN	PINA

#define DB_PIN8_P		PA5
#define DB_PIN8_DDR		DDRA
#define DB_PIN8_PORT	PORTA
#define DB_PIN8_PIN		PINA

#define DB_PIN6_P		PA7
#define DB_PIN6_DDR		DDRA
#define DB_PIN6_PORT	PORTA
#define DB_PIN6_PIN		PINA

#define DB_PIN2_P		PB0
#define DB_PIN2_DDR		DDRB
#define DB_PIN2_PORT	PORTB
#define DB_PIN2_PIN		PINB

#define DB_PIN3_P		PB1
#define DB_PIN3_DDR		DDRB
#define DB_PIN3_PORT	PORTB
#define DB_PIN3_PIN		PINB

#define DB_PIN5_P		PB2
#define DB_PIN5_DDR		DDRB
#define DB_PIN5_PORT	PORTB
#define DB_PIN5_PIN		PINB


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
 * DEV 0x30 PIN. PIN: 2-13
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

		case 5:
			DB_PIN5_DDR &= ~(1<<DB_PIN5_P);				// set as input
			data = DB_PIN5_PIN & (1<<DB_PIN5_P);		// read port and mask pin
			break;

		case 6:
			DB_PIN6_DDR &= ~(1<<DB_PIN6_P);				// set as input
			data = DB_PIN6_PIN & (1<<DB_PIN6_P);		// read port and mask pin
			break;

		case 8:
			DB_PIN8_DDR &= ~(1<<DB_PIN8_P);				// set as input
			data = DB_PIN8_PIN & (1<<DB_PIN8_P);		// read port and mask pin
			break;

		case 10:
			DB_PIN10_DDR &= ~(1<<DB_PIN10_P);				// set as input
			data = DB_PIN10_PIN & (1<<DB_PIN10_P);		// read port and mask pin
			break;

		case 11:
			DB_PIN11_DDR &= ~(1<<DB_PIN11_P);				// set as input
			data = DB_PIN11_PIN & (1<<DB_PIN11_P);		// read port and mask pin
			break;

		case 12:
			DB_PIN12_DDR &= ~(1<<DB_PIN12_P);				// set as input
			data = DB_PIN12_PIN & (1<<DB_PIN12_P);		// read port and mask pin
			break;

		case 13:
			DB_PIN13_DDR &= ~(1<<DB_PIN13_P);				// set as input
			data = DB_PIN13_PIN & (1<<DB_PIN13_P);		// read port and mask pin
			break;
	}

	// Convert to 0 or 1
	usiTwiTransmitByte( DEV_DEBUG_ID );
	(data == 0) ? (data = 0) : (data = 1);
	usiTwiTransmitByte( data );
}

/*
 * Command 0x40 -  Set IO PIN LOW
 * DEV 0x40 PIN. PIN: 2-13
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

		case 5:
			DB_PIN5_DDR |= (1<<DB_PIN5_P);				// set as output
			DB_PIN5_PORT &= ~(1<<DB_PIN5_P);			// set pin LOW
			break;

		case 6:
			DB_PIN6_DDR |= (1<<DB_PIN6_P);				// set as output
			DB_PIN6_PORT &= ~(1<<DB_PIN6_P);			// set pin LOW
			break;

		case 8:
			DB_PIN8_DDR |= (1<<DB_PIN8_P);				// set as output
			DB_PIN8_PORT &= ~(1<<DB_PIN8_P);			// set pin LOW
			break;

		case 10:
			DB_PIN10_DDR |= (1<<DB_PIN10_P);				// set as output
			DB_PIN10_PORT &= ~(1<<DB_PIN10_P);			// set pin LOW
			break;

		case 11:
			DB_PIN11_DDR |= (1<<DB_PIN11_P);				// set as output
			DB_PIN11_PORT &= ~(1<<DB_PIN11_P);			// set pin LOW
			break;

		case 12:
			DB_PIN12_DDR |= (1<<DB_PIN12_P);				// set as output
			DB_PIN12_PORT &= ~(1<<DB_PIN12_P);			// set pin LOW
			break;

		case 13:
			DB_PIN13_DDR |= (1<<DB_PIN13_P);				// set as output
			DB_PIN13_PORT &= ~(1<<DB_PIN13_P);			// set pin LOW
			break;
	}
}

/*
 * Command 0x40 -  Set IO PIN LOW
 * DEV 0x40 PIN. PIN: 2-13
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

		case 5:
			DB_PIN5_DDR |= (1<<DB_PIN5_P);			// set as output
			DB_PIN5_PORT |= (1<<DB_PIN5_P);			// set pin HIGH
			break;

		case 6:
			DB_PIN6_DDR |= (1<<DB_PIN6_P);			// set as output
			DB_PIN6_PORT |= (1<<DB_PIN6_P);			// set pin HIGH
			break;

		case 8:
			DB_PIN8_DDR |= (1<<DB_PIN8_P);			// set as output
			DB_PIN8_PORT |= (1<<DB_PIN8_P);			// set pin HIGH
			break;

		case 10:
			DB_PIN10_DDR |= (1<<DB_PIN10_P);			// set as output
			DB_PIN10_PORT |= (1<<DB_PIN10_P);			// set pin HIGH
			break;

		case 11:
			DB_PIN11_DDR |= (1<<DB_PIN11_P);			// set as output
			DB_PIN11_PORT |= (1<<DB_PIN11_P);			// set pin HIGH
			break;

		case 12:
			DB_PIN12_DDR |= (1<<DB_PIN12_P);			// set as output
			DB_PIN12_PORT |= (1<<DB_PIN12_P);			// set pin HIGH
			break;

		case 13:
			DB_PIN13_DDR |= (1<<DB_PIN13_P);			// set as output
			DB_PIN13_PORT |= (1<<DB_PIN13_P);			// set pin HIGH
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
		case DB_VREF_1_1:
			ADMUX = (1<<REFS1)|(0<<REFS0)|(chan & 0x07);	// Internal Vref = 1.1v, Left Adj, lower 3 bits select channel.
			break;
		case DB_VREF_EXT:
			ADMUX = (0<<REFS1)|(1<<REFS0)|(chan & 0x07);	// Vref = Ext, Left Adj, lower 3 bits select channel.
			break;
		case DB_VREF_VCC:
		default:
			ADMUX = (0<<REFS1)|(0<<REFS0)|(chan & 0x07);	// Internal Vref = Vcc, Left Adj, lower 3 bits select channel.
			break;
	}
	// Enable ADC, set clock rate
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);	// ADC Enable, CPU/64 clock (8MHz => 125kHz or ~120us/sample)
	// Set control
	ADCSRB = (1<<ADLAR);

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
 * Loads output FIFO with ADC channel data.
 *
 * NOTE: ADC4 and ADC6 are used by the I2C interface.
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
 * Command 0x79 - Set Vref to Internal 1.1v
 */
void dev_debug_set_vref_1_1()
{
	debug_adc_vref = DB_VREF_1_1;
}

/*
 * Command 0x7A - Set Vref to Ext
 */
void dev_debug_set_vref_ext()
{
	debug_adc_vref = DB_VREF_EXT;
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
