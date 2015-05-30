/*
 * dev_led_1.h
 *
 * Created: 5/19/2015 1:09:53 PM
 *  Author: Chip
 */ 


#ifndef DEV_LED_1_H_
#define DEV_LED_1_H_

// LED 1
#define DEV_LED_1_ID 250

#define DEV_LED_DDR DDRB
#define DEV_LED_PORT PORTB
#define DEV_LED_OUT_PIN	PB1


void dev_led_1_init();
void dev_led_1_off();
void dev_led_1_on();


#endif /* DEV_LED_1_H_ */