/*
 * dev_adc_1.h
 *
 * Created: 5/19/2015 6:05:26 PM
 *  Author: Chip
 */ 


#ifndef DEV_ADC_1_H_
#define DEV_ADC_1_H_


// ADC 1
#define DEV_ADC_1_ID 1

void dev_adc_1_init();
void dev_adc_1_service();
void dev_adc_1_get_data();
void dev_adc_1_set_gain();
void dev_adc_1_set_reference();


#endif /* DEV_ADC_1_H_ */