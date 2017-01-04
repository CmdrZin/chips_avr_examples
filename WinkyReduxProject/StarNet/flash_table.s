/*
 * flash_table.s
 *
 * Created: 11/25/2015	0.01	ndp
 *  Author: Chip
 *
 * Data format utilites for pulling data from data structures in FLASH memory.
 *
 */ 

 #include <avr/io.h>

.global flash_get_ddr_pattern
/*
 * r25:r24 = index (H,L)
 * ddrTable has 1 byte per entry.
 * returns (uint8_t)r24 = ddrTable[index]
 * Z = r31:30
 *
 */
 flash_get_ddr_pattern:
	; Z = table
	ldi		r31, hi8((ddrTable))
	ldi		r30, lo8((ddrTable))
	; add index
	add		r30, r24
	adc		r31, r1					; r1 always 0
	;
	lpm		r24, Z					; get seg
	;
	mov		r25, r1					; set to 0
	;
	ret

.global flash_get_port_pattern
/*
 * r25:r24 = index (H,L)
 * portTable has 1 byte per entry.
 * returns (uint8_t)r24 = portTable[index]
 * Z = r31:30
 *
 */
 flash_get_port_pattern:
	; Z = table
	ldi		r31, hi8((portTable))
	ldi		r30, lo8((portTable))
	; add index
	add		r30, r24
	adc		r31, r1					; r1 always 0
	;
	lpm		r24, Z					; get seg
	;
	mov		r25, r1					; set to 0
	;
	ret

.global flash_get_led_index
/*
 * r25:r24 = index (H,L)
 * ledTable has 1 byte per entry.
 * returns (uint8_t)r24 = ledTable[index]
 * Z = r31:30
 *
 */
 flash_get_led_index:
	; Z = table
	ldi		r31, hi8((ledTable))
	ldi		r30, lo8((ledTable))
	; add index
	add		r30, r24
	adc		r31, r1					; r1 always 0
	;
	lpm		r24, Z					; get seg
	;
	mov		r25, r1					; set to 0
	;
	ret
