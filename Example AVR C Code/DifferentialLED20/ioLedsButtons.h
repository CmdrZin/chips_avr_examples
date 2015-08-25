/*
 * ioLedsButtons.h
 *
 * Created: 8/25/2015 12:28:17 PM
 *  Author: Chip
 */ 


#ifndef IOLEDSBUTTONS_H_
#define IOLEDSBUTTONS_H_

#include <stdbool.h>

typedef enum { IDLE, PEAK, POWER } IO_DIFF_LED_MODE;

void io_init();
void io_service();

IO_DIFF_LED_MODE ilb_servicePeak();
IO_DIFF_LED_MODE ilb_servicePower();

void io_setLedPeak( bool val );
void io_setLedPower( bool val );

void io_turnDiffLedOn( uint8_t led );



#endif /* IOLEDSBUTTONS_H_ */