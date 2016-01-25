/*
 * led_display.h
 *
 * Created: 1/13/2016 1:04:23 PM
 *  Author: Chip
 */ 


#ifndef LED_DISPLAY_H_
#define LED_DISPLAY_H_

void ld_init();
void ld_service();
void ld_loadIcon( uint16_t val, uint8_t b );
void ld_directLoadIcon( uint8_t *buff );

#endif /* LED_DISPLAY_H_ */