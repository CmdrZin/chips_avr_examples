/*
 * flash_table.s
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 * Data format utilites for pulling data from data structures in FLASH memory.
 *
 */ 

 #include <avr/io.h>

.global flash_get_mod_access_id
/*
 * r25:r24 = index (H,L)
 * mod_access_table has 4 bytes per entry. (see sysdefs.h MOD_ACCESS_ENTRY)
 * returns (uint8_t)r25:24 = mod_access_table[index].id
 * Z = r31:30
 *
 * NOTE: If this CPU had a MUL instruction, the sizeof element could be passed in for index adjustment.
 */
 flash_get_mod_access_id:
	; multiply index by 4
	mov		r18, r24
	add		r24, r24			; double
	add		r24, r24			; x4
	; Z = table
	ldi		r31, hi8((mod_access_table))
	ldi		r30, lo8((mod_access_table))
	; add index
	add		r30, r24
	adc		r31, r1					; r1 always 0
	; get id
	lpm		r24, Z
	mov		r25, r1
	;
	ret

.global flash_get_mod_function_table
/*
 * r25:r24 = index
 * mod_access_table has 4 bytes per entry. (see sysdefs.h MOD_ACCESS_ENTRY)
 * returns (uint16_t)r25:24 = mod_access_table[index].cmd_table
 *
 * NOTE: If this CPU had a MUL instruction, the sizeof element could be passed in for index adjustment.
 */
 flash_get_mod_function_table:
	; multiply index by 4
	mov		r18, r24
	add		r24, r24			; double
	add		r24, r24			; x4
	; Z = table
	ldi		r31, hi8((mod_access_table))
	ldi		r30, lo8((mod_access_table))
	; add index
	add		r30, r24
	adc		r31, r1					; r1 always 0
	; add offset to cmd table
	adiw	r30, 2
	; get cmd table
	lpm		r24, Z+
	lpm		r25, Z
	;
	ret

.global flash_get_access_cmd
/*
 * r25:24	= index
 * r23:22	= table
 * table has 4 bytes per entry. A Key and a Value (see sysdefs.h MOD_FUNCTION_ENTRY)
 * returns (uint16_t)r25:24 table[index].id
 *
 * NOTE: If this CPU had a MUL instruction, the sizeof element could be passed in for index adjustment.
 */
 flash_get_access_cmd:
	; multiply index by 4
	add		r24, r24			; double
	add		r24, r24			; x4
	; Z = table
	mov		r30, r22
	mov		r31, r23
	; add index
	add		r30, r24
	adc		r31, r1					; r1 always 0
	; get key
	lpm		r24, Z+
	lpm		r25, Z
	;
	ret

.global flash_get_access_func
/*
 * r25:24	= index
 * r23:22	= table
 * table has 4 bytes per entry. A Key and a Value (see sysdefs.h MOD_FUNCTION_ENTRY)
 * returns (uint16_t)r25:24 table[index].func
 *
 * NOTE: If this CPU had a MUL instruction, the sizeof element could be passed in for index adjustment.
 */
 flash_get_access_func:
	; multiply index by 4
	add		r24, r24			; double
	add		r24, r24			; x4
	; Z = table
	mov		r30, r22
	mov		r31, r23
	; add index
	add		r30, r24
	adc		r31, r1					; r1 always 0
	; add offset to value
	adiw	r30, 2
	; get key
	lpm		r24, Z+
	lpm		r25, Z
	;
	ret