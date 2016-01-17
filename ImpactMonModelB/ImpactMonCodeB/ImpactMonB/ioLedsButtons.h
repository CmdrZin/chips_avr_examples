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

void idl_service();

void idl_setBarMode( IO_DIFF_LED_MODE mode );
void idl_setPeakValue( uint8_t val );
void idl_setPowerValue( uint8_t val );

void ilb_servicePeak();
void ilb_servicePower();

void io_setLedPeak( bool val );
void io_setLedPower( bool val );

void io_turnDiffLedOn( uint8_t led );



#endif /* IOLEDSBUTTONS_H_ */