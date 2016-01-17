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

.global flash_get_seg_led
/*
 * r25:r24 = index (H,L)
 * ilb_segTable has 1 byte per entry.
 * returns (uint8_t)r24 = ilb_segTable[index]
 * Z = r31:30
 *
 */
 flash_get_seg_led:
	; Z = table
	ldi		r31, hi8((ilb_segTable))
	ldi		r30, lo8((ilb_segTable))
	; add index
	add		r30, r24
	adc		r31, r1					; r1 always 0
	;
	lpm		r24, Z					; get seg
	;
	mov		r25, r1					; set to 0
	;
	ret
