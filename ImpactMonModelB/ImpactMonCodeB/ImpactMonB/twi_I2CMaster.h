/*
 * twi_I2CMaster.h
 *
 * Created: 8/27/2015 3:30:49 PM
 *  Author: Chip
 */ 


#ifndef TWI_I2CMASTER_H_
#define TWI_I2CMASTER_H_

#include <avr/io.h>
#include <stdbool.h>

void tim_init();

void tim_write( uint8_t sla, uint8_t* buffer, uint8_t nbytes );
bool tim_isBusy();
bool tim_hasData();
void tim_read( uint8_t sla, uint8_t nbytes );
uint8_t tim_readData();



#endif /* TWI_I2CMASTER_H_ */