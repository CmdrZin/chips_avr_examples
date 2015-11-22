/*
 * mod_adc.h
 *
 * Created: 10/20/2015		0.01	ndp
 *  Author: Nels "Chip" Pearson
 */ 


#ifndef MOD_ADC_H_
#define MOD_ADC_H_


// ADC
void mod_adc_init();
uint8_t mod_adc_read( uint8_t chan );


#endif /* MOD_ADC_H_ */