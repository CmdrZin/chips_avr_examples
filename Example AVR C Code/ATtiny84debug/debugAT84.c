/*
 * debugAT84.c
 *
 * Created: 6/8/2015 9:54:34 PM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "sysTimer.h"
// DEV_1MS_TIC, 

#include "usiTwiSlave.h"
#include "access.h"

#include "debugAT84.h"

#define DB_PIN1_P		PA0
#define DB_PIN1_DDR		DDRA
#define DB_PIN1_PORT	PORTA
#define DB_PIN1_PIN		PINA

#define DB_PIN2_P		PA1
#define DB_PIN2_DDR		DDRA
#define DB_PIN2_PORT	PORTA
#define DB_PIN2_PIN		PINA

#define DB_PIN3_P		PA2
#define DB_PIN3_DDR		DDRA
#define DB_PIN3_PORT	PORTA
#define DB_PIN3_PIN		PINA

#define DB_PIN4_P		PA3
#define DB_PIN4_DDR		DDRA
#define DB_PIN4_PORT	PORTA
#define DB_PIN4_PIN		PINA

#define DB_PIN5_P		PB0
#define DB_PIN5_DDR		DDRB
#define DB_PIN5_PORT	PORTB
#define DB_PIN5_PIN		PINB

#define DB_PIN6_P		PB1
#define DB_PIN6_DDR		DDRB
#define DB_PIN6_PORT	PORTB
#define DB_PIN6_PIN		PINB

#define DB_PIN7_P		PB2
#define DB_PIN7_DDR		DDRB
#define DB_PIN7_PORT	PORTB
#define DB_PIN7_PIN		PINB

#define DB_PIN8_P		PA5
#define DB_PIN8_DDR		DDRA
#define DB_PIN8_PORT	PORTA
#define DB_PIN8_PIN		PINA

#define DB_PIN9_P		PA7
#define DB_PIN9_DDR		DDRA
#define DB_PIN9_PORT	PORTA
#define DB_PIN9_PIN		PINA


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
	// Set defaults. These may come from EEPROM later.
	debug_adc_vref = DB_VREF_VCC;
}

void dev_debug_service()
{
	// NO dynamic services at this time.
	return;
}


/* *** DEBUG ACCESS FUNCTIONS *** */

/*
 * Command 0x30 -  Read IO PIN
 * DEV 0x30 PIN. PIN: 1-9
 */
void dev_debug_read_pin()
{
	uint8_t pin = getCmdData(2);
	uint8_t data = 0;

	switch(pin) {
		case 1:
			DB_PIN1_DDR &= ~(1<<DB_PIN1_P);				// set as input
			data = DB_PIN1_PIN & (1<<DB_PIN1_P);		// read port and mask pin
			break;

		case 2:
			DB_PIN2_DDR &= ~(1<<DB_PIN2_P);				// set as input
			data = DB_PIN2_PIN & (1<<DB_PIN2_P);		// read port and mask pin
			break;

		case 3:
			DB_PIN3_DDR &= ~(1<<DB_PIN3_P);				// set as input
			data = DB_PIN3_PIN & (1<<DB_PIN3_P);		// read port and mask pin
			break;

		case 4:
			DB_PIN4_DDR &= ~(1<<DB_PIN4_P);				// set as input
			data = DB_PIN4_PIN & (1<<DB_PIN4_P);		// read port and mask pin
			break;

		case 5:
			DB_PIN5_DDR &= ~(1<<DB_PIN5_P);				// set as input
			data = DB_PIN5_PIN & (1<<DB_PIN5_P);		// read port and mask pin
			break;

		case 6:
			DB_PIN6_DDR &= ~(1<<DB_PIN6_P);				// set as input
			data = DB_PIN6_PIN & (1<<DB_PIN6_P);		// read port and mask pin
			break;

		case 7:
			DB_PIN7_DDR &= ~(1<<DB_PIN7_P);				// set as input
			data = DB_PIN7_PIN & (1<<DB_PIN7_P);		// read port and mask pin
			break;

		case 8:
			DB_PIN8_DDR &= ~(1<<DB_PIN8_P);				// set as input
			data = DB_PIN8_PIN & (1<<DB_PIN8_P);		// read port and mask pin
			break;

		case 9:
			DB_PIN9_DDR &= ~(1<<DB_PIN9_P);				// set as input
			data = DB_PIN9_PIN & (1<<DB_PIN9_P);		// read port and mask pin
			break;
	}

	// Convert to 0 or 1
	usiTwiTransmitByte( DEV_DEBUG_ID );
	(data == 0) ? (data = 0) : (data = 1);
	usiTwiTransmitByte( data );
}

/*
 * Command 0x40 -  Set IO PIN LOW
 * DEV 0x40 PIN. PIN: 1-9
 */
void dev_debug_set_pin_low()
{
	uint8_t pin = getCmdData(2);
	debug_set_pin_low(pin);
}

void debug_set_pin_low(uint8_t pin)
{
	switch(pin) {
		case 1:
			DB_PIN1_DDR |= (1<<DB_PIN1_P);				// set as output
			DB_PIN1_PORT &= ~(1<<DB_PIN1_P);			// set pin LOW
			break;

		case 2:
			DB_PIN2_DDR |= (1<<DB_PIN2_P);				// set as output
			DB_PIN2_PORT &= ~(1<<DB_PIN2_P);			// set pin LOW
			break;

		case 3:
			DB_PIN3_DDR |= (1<<DB_PIN3_P);				// set as output
			DB_PIN3_PORT &= ~(1<<DB_PIN3_P);			// set pin LOW
			break;

		case 4:
			DB_PIN4_DDR |= (1<<DB_PIN4_P);				// set as output
			DB_PIN4_PORT &= ~(1<<DB_PIN4_P);			// set pin LOW
			break;

		case 5:
			DB_PIN5_DDR |= (1<<DB_PIN5_P);				// set as output
			DB_PIN5_PORT &= ~(1<<DB_PIN5_P);			// set pin LOW
			break;

		case 6:
			DB_PIN6_DDR |= (1<<DB_PIN6_P);				// set as output
			DB_PIN6_PORT &= ~(1<<DB_PIN6_P);			// set pin LOW
			break;

		case 7:
			DB_PIN7_DDR |= (1<<DB_PIN7_P);				// set as output
			DB_PIN7_PORT &= ~(1<<DB_PIN7_P);			// set pin LOW
			break;

		case 8:
			DB_PIN8_DDR |= (1<<DB_PIN8_P);				// set as output
			DB_PIN8_PORT &= ~(1<<DB_PIN8_P);			// set pin LOW
			break;

		case 9:
			DB_PIN9_DDR |= (1<<DB_PIN9_P);				// set as output
			DB_PIN9_PORT &= ~(1<<DB_PIN9_P);			// set pin LOW
			break;
	}
}

/*
 * Command 0x40 -  Set IO PIN LOW
 * DEV 0x40 PIN. PIN: 1-9
 */
void dev_debug_set_pin_high()
{
	uint8_t pin = getCmdData(2);
	debug_set_pin_high(pin);
}

void debug_set_pin_high(uint8_t pin)
{
	switch(pin) {
		case 1:
			DB_PIN1_DDR |= (1<<DB_PIN1_P);			// set as output
			DB_PIN1_PORT |= (1<<DB_PIN1_P);			// set pin HIGH
			break;

		case 2:
			DB_PIN2_DDR |= (1<<DB_PIN2_P);			// set as output
			DB_PIN2_PORT |= (1<<DB_PIN2_P);			// set pin HIGH
			break;

		case 3:
			DB_PIN3_DDR |= (1<<DB_PIN3_P);			// set as output
			DB_PIN3_PORT |= (1<<DB_PIN3_P);			// set pin HIGH
			break;

		case 4:
			DB_PIN4_DDR |= (1<<DB_PIN4_P);			// set as output
			DB_PIN4_PORT |= (1<<DB_PIN4_P);			// set pin HIGH
			break;

		case 5:
			DB_PIN5_DDR |= (1<<DB_PIN5_P);			// set as output
			DB_PIN5_PORT |= (1<<DB_PIN5_P);			// set pin HIGH
			break;

		case 6:
			DB_PIN6_DDR |= (1<<DB_PIN6_P);			// set as output
			DB_PIN6_PORT |= (1<<DB_PIN6_P);			// set pin HIGH
			break;

		case 7:
			DB_PIN7_DDR |= (1<<DB_PIN7_P);			// set as output
			DB_PIN7_PORT |= (1<<DB_PIN7_P);			// set pin HIGH
			break;

		case 8:
			DB_PIN8_DDR |= (1<<DB_PIN8_P);			// set as output
			DB_PIN8_PORT |= (1<<DB_PIN8_P);			// set pin HIGH
			break;

		case 9:
			DB_PIN9_DDR |= (1<<DB_PIN9_P);			// set as output
			DB_PIN9_PORT |= (1<<DB_PIN9_P);			// set pin HIGH
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
 * Command 0xA1 -  Pulse PIN 1 for N ms
 * DEV 0xA1 MS
 */
void dev_debug_pulse_pin1()
{
	dev_pulse_pin(1, getCmdData(2));
}

/*
 * Command 0xA2 -  Pulse PIN 2 for N ms
 * DEV 0xA2 MS
 */
void dev_debug_pulse_pin2()
{
	dev_pulse_pin(2, getCmdData(2));
}

/*
 * Command 0xA3 -  Pulse PIN 3 for N ms
 * DEV 0xA3 MS
 */
void dev_debug_pulse_pin3()
{
	dev_pulse_pin(3, getCmdData(2));
}

/*
 * Command 0xA4 -  Pulse PIN 4 for N ms
 * DEV 0xA4 MS
 */
void dev_debug_pulse_pin4()
{
	dev_pulse_pin(4, getCmdData(2));
}

/*
 * Command 0xA5 -  Pulse PIN 5 for N ms
 * DEV 0xA5 MS
 */
void dev_debug_pulse_pin5()
{
	dev_pulse_pin(5, getCmdData(2));
}

/*
 * Command 0xA6 -  Pulse PIN 6 for N ms
 * DEV 0xA6 MS
 */
void dev_debug_pulse_pin6()
{
	dev_pulse_pin(6, getCmdData(2));
}

/*
 * Command 0xA7 -  Pulse PIN 7 for N ms
 * DEV 0xA7 MS
 */
void dev_debug_pulse_pin7()
{
	dev_pulse_pin(7, getCmdData(2));
}

/*
 * Command 0xA8 -  Pulse PIN 8 for N ms
 * DEV 0xA8 MS
 */
void dev_debug_pulse_pin8()
{
	dev_pulse_pin(8, getCmdData(2));
}

/*
 * Command 0xA9 -  Pulse PIN 9 for N ms
 * DEV 0xA9 MS
 */
void dev_debug_pulse_pin9()
{
	dev_pulse_pin(9, getCmdData(2));
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
 * Command 0x70 -  Read ADC0
 * Loads output FIFO with ADC channel 0 data.
 */
void dev_debug_get_adc0()
{
	dev_adc_trigger(0);
}

/*
 * Command 0x71 -  Read ADC1
 * Loads output FIFO with ADC channel 1 data.
 */
void dev_debug_get_adc1()
{
	dev_adc_trigger(1);
}

/*
 * Command 0x72 -  Read ADC2
 * Loads output FIFO with ADC channel 2 data.
 */
void dev_debug_get_adc2()
{
	dev_adc_trigger(2);
}

/*
 * Command 0x73 -  Read ADC3
 * Loads output FIFO with ADC channel 3 data.
 */
void dev_debug_get_adc3()
{
	dev_adc_trigger(3);
}

/*
 * Command 0x75 -  Read ADC5
 * Loads output FIFO with ADC channel 5 data.
 */
void dev_debug_get_adc5()
{
	dev_adc_trigger(5);
}

/*
 * Command 0x77 -  Read ADC7
 * Loads output FIFO with ADC channel 7 data.
 */
void dev_debug_get_adc7()
{
	dev_adc_trigger(7);
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

