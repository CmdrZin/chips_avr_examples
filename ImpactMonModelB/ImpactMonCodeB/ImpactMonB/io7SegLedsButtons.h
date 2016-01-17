/*
 * ioLedsButtons.h
 *
 * Created: 8/25/2015 12:28:17 PM
 *  Author: Chip
 */ 


#ifndef IOLEDSBUTTONS_H_
#define IOLEDSBUTTONS_H_

#include <stdbool.h>

typedef enum {IDL_OFF, IDL_DL1, IDL_DL2 } IDL_DISPLAY;

void idl_init();
void idl_service();

void idl_setMode( IDL_DISPLAY mode );
void idl_setValue( uint16_t val );
void idl_displayDigit( uint8_t val, IDL_DISPLAY digit );
void idl_setDecimalPoints(uint8_t val);
uint8_t idl_scanButtons();

#endif /* IOLEDSBUTTONS_H_ */