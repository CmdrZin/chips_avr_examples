/*
 * debugAT84.h
 *
 * Created: 6/8/2015 9:54:55 PM
 *  Author: Chip
 */ 


#ifndef DEBUGAT84_H_
#define DEBUGAT84_H_

#define DEV_DEBUG_ID 255

typedef enum { DB_VREF_VCC, DB_VREF_1_1, DB_VREF_EXT } DEBUG_VREF;

void dev_debug_init();
void dev_debug_service();
void dev_debug_read_pin();
void dev_debug_set_pin_low();
void debug_set_pin_low(uint8_t pin);
void dev_debug_set_pin_high();
void debug_set_pin_high(uint8_t pin);

void dev_debug_get_adc();
void dev_debug_set_vref_vcc();
void dev_debug_set_vref_1_1();
void dev_debug_set_vref_ext();

void dev_debug_pulse_pin();

void dev_debug_get_build_time();
void dev_debug_get_build_date();
void dev_debug_get_version();


#endif /* DEBUGAT84_H_ */