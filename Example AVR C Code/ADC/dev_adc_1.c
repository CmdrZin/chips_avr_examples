/*
 * dev_adc_1.c
 *
 * Created: 5/18/2015 8:58:48 PM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "sysTimer.h"
// DEV_ADC_1_10MS_TIC, 

#include "usiTwiSlave.h"

#include "dev_adc_1.h"

#define DEV_ADC_1_COUNT 10			// N * 10ms (default)


// Current output from ADC. Left justified.
static uint8_t dev_adc_1_H;
static uint8_t dev_adc_1_L;
static uint8_t dev_adc_1_delay;

uint8_t test;

/*
 * Initialize Hardware for ADC use
 * input reg:	none
 * output reg:	none
 *
 * Single Ended
 * Available channels 0,1,2,3
 * ATmega164P has Internal 2.56v reference. 20MHz
 * ATmega328P has Internal 1.1v reference. 8MHz
 * ATtiny85 has Internal 2.56v reference. 8MHz Cannot use 1.1v with I2C.
 *
 */
void dev_adc_1_init()
{
	ADMUX = (1<<REFS1)|(0<<REFS0)|(1<<ADLAR)|(1<<REFS2);	// Internal Vref = 2.56v, Left Adj, lower 4 bits select channel.
	// NOTE: Can not use internal 1.1V because it will takeover PB0 for use with bypass cap. ONLY 2.56V can be used.
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);	// ADC Enable, CPU/64 clock (8MHz => 125kHz or ~120us/sample)

	dev_adc_1_delay = DEV_ADC_1_COUNT;

	test = 0;

	return;
}

/*
 * Trigger ADC channel 0
 *
 * input:	none
 *
 * output:	dev_adc_1_L = ADCL
 * 			dev_adc_1_H = ADCH
 *
 * Called each 10ms from dev_service.
 *
 * NOTE: MUST read ADCL first then ADCH.
 */
void dev_adc_1_service()
{
	if( GPIOR0 & (1<<DEV_ADC_1_10MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_ADC_1_10MS_TIC);
		// check delay
		if( --dev_adc_1_delay == 0 )
		{
			dev_adc_1_delay = DEV_ADC_1_COUNT;
			// trigger ADC sample.
			ADMUX &= 0xF0;			// select channel 0. lower 4 bits select channel.
			ADMUX |= 0x02;			// use ADC2.
			ADCSRA |= (1<<ADSC);		// trigger
			// Wait ~120us
			while( !(ADCSRA & (1<<ADSC)) )
			{
				;
			}
			// Get Data
			dev_adc_1_L = ADCL;		// get 2 LSBs .. d7:6
			dev_adc_1_H = ADCH;		// get 8 MSBs .. d7:0..releases ADC data reg.
		}
	}
	
	return;
}

/*
 * Get ADC Data and put it into output fifo with device ID
 *
 * input:	none
 *
 * output:	data into Rx_buf
 *
 */
void dev_adc_1_get_data()
{
//	uint8_t data;
	usiTwiTransmitByte( DEV_ADC_1_ID );
//	data = dev_adc_1_H + test++;
	usiTwiTransmitByte( dev_adc_1_H );
//	usiTwiTransmitByte( data );
//	data = dev_adc_1_L + test++;
usiTwiTransmitByte( dev_adc_1_L );
//	usiTwiTransmitByte( data );

//	usiTwiTransmitByte( test++ );
//	usiTwiTransmitByte( test++ );
	
	return;
}

// NOT SUPPORTED
void dev_adc_1_set_gain()
{
	return;
}

// NOT SUPPORTED
void dev_adc_1_set_reference()
{
	return;
}

