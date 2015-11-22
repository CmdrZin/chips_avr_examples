/*
 * mod_led_status.h
 *
 * Created: 8/13/2015 4:34:10 PM
 *  Author: Chip
 */ 


#ifndef MOD_LED_STATUS_H_
#define MOD_LED_STATUS_H_

#include <stdbool.h>

#define MOD_LED_STATUS_ID	0x01

#define MLS_TURN_ON_LED			0x01
#define MLS_TURN_OFF_LED		0x02


void mod_led_status_init();
void mod_led_status_service();

void mod_led_status_on();
void mod_led_status_off();

void mls_setRangeStatus( uint8_t led, bool state );
void mls_turn_led_on( uint8_t val );
void mls_turn_led_off( uint8_t val );


#endif /* MOD_LED_STATUS_H_ */