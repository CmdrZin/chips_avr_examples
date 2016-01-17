/*
 * mod_UserIO.h
 *
 * Created: 8/30/2015 7:31:00 PM
 *  Author: Chip
 */ 


#ifndef MOD_USERIO_H_
#define MOD_USERIO_H_

typedef enum { MUIO_TEST, MUIO_RAW_SERIAL, MUIO_NORMAL, MUIO_RANGE } MUIO_DISPLAY_MODE;



void uio_init();
void uio_service();
void uio_setMode( MUIO_DISPLAY_MODE mode );

void muio_testMode();
void muio_serial_raw();
void muio_setPeak( uint16_t val );
void muio_normal();
void muio_range();
void muio_powerMode();


#endif /* MOD_USERIO_H_ */