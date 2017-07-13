/*
 * io_4x27SegLeds.h
 *
 * Created: 6/29/2017	0.01	ndp
 *  Author: Chip
 */ 


#ifndef IO_4x2SEGLEDS_H_
#define IO_4x2SEGLEDS_H_

#define ISL_DECIMAL_POINT	0x80
#define ISL_BLANK			0x10

void isl_init();
uint8_t isl_getPattern( uint8_t val );
void isl_update( uint8_t digit, uint8_t pattern );

#endif /* IO_4x2SEGLEDS_H_ */