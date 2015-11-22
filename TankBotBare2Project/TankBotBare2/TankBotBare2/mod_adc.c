/*
 * mod_adc.c
 *
 * Created: 10/20/2015		0.01	ndp
 *  Author: Nels "Chip" Pearson
 *
 * General ADC Support
 * Target: ATmega164P has Internal 2.56v reference. 20MHz
 *
 */ 

#include <avr/io.h>


// Current output from ADC. Left justified.
static uint8_t mod_adc_H;
static uint8_t mod_adc_L;


/*
 * Initialize Hardware for ADC use
 * input reg:	none
 * output reg:	none
 *
 * Single Ended
 * Available channels 0,1,2,3
 *
 */
void mod_adc_init()
{
	ADMUX = (1<<REFS1)|(1<<REFS0)|(1<<ADLAR);				// Internal Vref = 2.56v, Left Adj, lower 4 bits select channel.
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);	// ADC Enable, CPU/128 clock (20MHz => 156kHz or ~64us/sample)
}

/*
 * Read ADC channel
 *
 * input:	channel 0-3
 *
 * output:	dev_adc_1_H = ADCH
 *
 * NOTE: Blocking command (~65us)
 * NOTE: MUST read ADCL first then ADCH.
 */
uint8_t mod_adc_read( uint8_t chan )
{
	ADMUX &= 0xF0;			// select channel 0. lower 4 bits select channel.
	ADMUX |= chan;
	ADCSRA |= (1<<ADSC);		// trigger
	// Wait ~65us
	while( (ADCSRA & (1<<ADSC)) )
	{
		;
	}
	// Get Data
	mod_adc_L = ADCL;		// get 2 LSBs .. d7:6
	mod_adc_H = ADCH;		// get 8 MSBs .. d7:0..releases ADC data reg.
	
	return( mod_adc_H );
}
