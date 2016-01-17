/*
 * adxl345.h
 *
 * Created: 9/7/2015 3:53:35 PM
 *  Author: Chip
 */ 


#ifndef MOD_ADXL345_H_
#define MOD_ADXL345_H_

#define MADX_I2C_ADRS	0x53

void mod_adxl_init();

void mod_adxl_sendCmd( uint8_t reg, uint8_t val );
void mod_adxl_readReg( uint8_t reg, uint8_t* val );
void mod_adxl_readData( uint8_t reg, uint8_t* val );
void mod_adxl_setRange( uint8_t val );

#endif /* MOD_ADXL345_H_ */