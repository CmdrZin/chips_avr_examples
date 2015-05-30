/*
 * dev_led_1.c
 *
 * Created: 5/18/2015 8:59:27 PM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "dev_led_1.h"

void dev_led_1_init()
{
//	DEV_LED_DDR |= _BV(DEV_LED_OUT_PIN);			// set HIGH for output
	DEV_LED_DDR |= (1<<DEV_LED_OUT_PIN);			// set HIGH for output
	return;
}

void dev_led_1_off()
{
//	DEV_LED_PORT &= ~_BV(DEV_LED_OUT_PIN);			// set LOW
	DEV_LED_PORT &= ~(1<<DEV_LED_OUT_PIN);			// set LOW
	return;
}

void dev_led_1_on()
{
//	DEV_LED_PORT |= _BV(DEV_LED_OUT_PIN);			// set HIGH
	DEV_LED_PORT |= (1<<DEV_LED_OUT_PIN);			// set HIGH
	return;
}
