/*
 * flash_table.h
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 */ 


#ifndef FLASH_TABLE_H_
#define FLASH_TABLE_H_


uint8_t flash_get_mod_access_id(uint8_t index);
MOD_FUNCTION_ENTRY* flash_get_mod_function_table(uint8_t index);

uint16_t flash_get_access_cmd(uint8_t index, MOD_FUNCTION_ENTRY* table);
uint16_t flash_get_access_func(uint8_t index, MOD_FUNCTION_ENTRY* table);


#endif /* FLASH_TABLE_H_ */