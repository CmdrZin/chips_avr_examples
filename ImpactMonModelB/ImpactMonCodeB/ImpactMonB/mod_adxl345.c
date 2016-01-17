/*
 * mod_adxl345.c
 *
 * Created: 9/7/2015 3:53:20 PM
 *  Author: Chip
 *
 * ADXL345 interface
 *
 *
 */ 

#include <avr/io.h>

#include "mod_adxl345.h"
#include "twi_I2CMaster.h"



#define	MADX_DEVID		0x00		// 0xE5 returned

#define	MADX_BW_RATE	0x2C
//	b7:4 = 0
//	b3:0 = Rate
#define MADX_BW_400		0x0C
#define MADX_BW_200		0x0B
#define MADX_BW_100		0x0A

#define	MADX_POWER_CTL	0x2D
//	b3 = Measure
#define	MADX_POWER_ON	0x08

#define	MADX_DATA_FORMAT	0x31
//	b3 = FULL_RES
//	b2 = Justify 0=Right, 1=Left
//	b1:0 = Range 00 2g, 01 4g, 10 8g, 11 16g
#define	MADX_DATA_FULL		0x08
#define	MADX_DATA_LEFT		0x04
#define	MADX_DATA_2G		0x00
#define	MADX_DATA_4G		0x01
#define	MADX_DATA_8G		0x02
#define	MADX_DATA_16G		0x03

#define	MADX_DATAZ0		0x36
#define	MADX_DATAZ1		0x37

#define	MADX_FIFO_CTL	0x38
//	wb7:6 = Mode	00 Bypass, 01 Gather, 10 Overwrite, 11 Trigger
#define	MADX_FIFO_BYP	0x00
#define	MADX_FIFO_GATH	0x40
#define	MADX_FIFO_OVW	0x80
#define	MADX_FIFO_TRIG	0xC0
//	rb4:0 = Samples

#define	MADX_FIFO_STATUS	0x39
//	r5:0 = Values available

uint8_t madx_buffer[3];

/*
 * mod_adxl_init()
 *
 * Set rate to 200 Hz
 * Set format for FULL, Right, 16g
 * Set FIFO for Overwrite
 * Set Power control to Measure
 */
void mod_adxl_init()
{
		madx_buffer[0] = MADX_BW_RATE;
		madx_buffer[1] = MADX_BW_200;						// 200 Hz.
		tim_write( MADX_I2C_ADRS, madx_buffer, 2 );

		while( tim_isBusy() );
		
		madx_buffer[0] = MADX_DATA_FORMAT;
		madx_buffer[1] = MADX_DATA_2G;
		tim_write( MADX_I2C_ADRS, madx_buffer, 2 );

		while( tim_isBusy() );

		madx_buffer[0] = MADX_FIFO_CTL;
		madx_buffer[1] = MADX_FIFO_OVW;						// Overwrite old data.
		tim_write( MADX_I2C_ADRS, madx_buffer, 2 );

		while( tim_isBusy() );

		madx_buffer[0] = MADX_POWER_CTL;
		madx_buffer[1] = MADX_POWER_ON;						// Enable measurements.
		tim_write( MADX_I2C_ADRS, madx_buffer, 2 );

		while( tim_isBusy() );
}

/*
 * SDA_W REG DATA
 */
void mod_adxl_sendCmd( uint8_t reg, uint8_t val )
{
	
}

/*
 * SDA_W REG wait SDA_R DATA
 */
void mod_adxl_readReg( uint8_t reg, uint8_t* val )
{
	
}

/*
 * Read 2 bytes of data
 *	SDA_W REG SDA_R DATA1 DATA2
 * NOTE: BLOCKING command. (~0.5ms)
 */
void mod_adxl_readData( uint8_t reg, uint8_t* val )
{
	madx_buffer[0] = reg;
	tim_write( MADX_I2C_ADRS, madx_buffer, 1 );

	while( tim_isBusy() );

	tim_read( MADX_I2C_ADRS, 2);	// Read N bytes.
	
	while( !tim_hasData() );		// wait for data.
	val[0] = tim_readData();		// read first byte
	
	while( !tim_hasData() );		// wait for data.
	val[1] = tim_readData();		// read second byte
}

/*
 * Set Range 1:1g, 2:2g, 3:4g, 4:8g, 5:16g
 * SDA_W REG DATA
 */
void mod_adxl_setRange( uint8_t val )
{
	madx_buffer[0] = MADX_DATA_FORMAT;
	
	switch( val )
	{
		case 1:
			madx_buffer[1] = MADX_DATA_2G;
			break;
			
		case 2:
			madx_buffer[1] = MADX_DATA_4G;
			break;

		case 3:
			madx_buffer[1] = MADX_DATA_8G;
			break;

		case 4:
			madx_buffer[1] = MADX_DATA_16G;
			break;
			
		default:
			madx_buffer[1] = MADX_DATA_16G;
			break;
	}
	
	tim_write( MADX_I2C_ADRS, madx_buffer, 2 );
}
