/*
 * led_display.h
 *
 * Created: 1/13/2016	0.01	ndp
 *  Author: Chip
 * revision: 2/01/2016	0.02	ndp	Added support for Marquee scroll.
 */ 


#ifndef LED_DISPLAY_H_
#define LED_DISPLAY_H_

void ld_init();
void ld_service();
void ld_loadIcon( uint16_t val, uint8_t b );
void ld_directLoadIcon( uint8_t *buff );
uint8_t ld_getFramesDisplayed();

#endif /* LED_DISPLAY_H_ */