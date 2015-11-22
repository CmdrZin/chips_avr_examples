/*
 * eeprom_util.h
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 * Based on example code from ATtiny84 data sheet.
 *
 */ 


#ifndef EEPROM_UTIL_H_
#define EEPROM_UTIL_H_

/* *** EEPROM USAGE MAP *** */
#define	PARAM_TABLE			0x0020		// reserve first 32 bytes for special use???
#define	PARAM_TABLE_SIZE	0x0080		// carve out 128 bytes. Leave the rest for CAL tables and such.


#define PARAM_NO_KEY	0x0100		// No key found

void EEPROM_write(unsigned int ucAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int ucAddress);

uint16_t param_get_value(uint8_t key);
void param_update(uint8_t key, uint8_t value);

void param_set_end(void);
void param_clear_all(void);

#endif /* EEPROM_UTIL_H_ */