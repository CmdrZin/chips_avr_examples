/*
 * flash_table.h
 *
 * Created: 11/25/2015	0.01	ndp
 *  Author: Chip
 */ 


#ifndef FLASH_TABLE_H_
#define FLASH_TABLE_H_

uint8_t flash_get_ddr_pattern(uint8_t index);
uint8_t flash_get_port_pattern(uint8_t index);
uint8_t flash_get_led_index(uint8_t index);


#endif /* FLASH_TABLE_H_ */