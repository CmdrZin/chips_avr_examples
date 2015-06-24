/*
 * eeprom_util.h
 *
 * Created: 6/22/2015 12:44:53 PM
 *  Author: Chip
 *
 * Based on example code from ATtiny84 data sheet.
 *
 */ 


#ifndef EEPROM_UTIL_H_
#define EEPROM_UTIL_H_

#define PARAM_NO_KEY	0x0100		// No key found

#define PRDCNT_RES	01				// RES: Period counter time resolution
									// RES=1 (0.125us), RES=2 (1.0us)[default], RES=3 (8.0us)
#define PRDCNT_MODE	02				// M: Period counter Mode
									// M=1 (continuous)[default], M=2 (Read once)
#define SOL_1_DRV	10				// Solenoid 1 drive direction
#define SOL_1_PULSE	11				// Solenoid 1 pulse length. N * 1 ms
#define SOL_2_DRV	20				// Solenoid 2 drive direction
#define SOL_2_PULSE	21				// Solenoid 2 pulse length. N * 1 ms


void EEPROM_write(unsigned int ucAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int ucAddress);

uint16_t param_get_value(uint8_t key);
void param_update(uint8_t key, uint8_t value);

#endif /* EEPROM_UTIL_H_ */