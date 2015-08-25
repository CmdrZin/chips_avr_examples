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

.global flash_get_diff_led
/*
 * r25:r24 = index (H,L)
 * ilb_diffLedTable has 2 bytes per entry. (see sysdefs.h ILB_DIFF_LED)
 * returns (uint8_t)r24 = ilb_diffLedTable[index].ddr
 * returns (uint8_t)r25 = ilb_diffLedTable[index].p
 * Z = r31:30
 *
 */
 flash_get_diff_led:
	; multiply index by 2
	mov		r18, r24
	add		r24, r24			; double
	; Z = table
	ldi		r31, hi8((ilb_diffLedTable))
	ldi		r30, lo8((ilb_diffLedTable))
	; add index
	add		r30, r24
	adc		r31, r1					; r1 always 0
	;
	lpm		r24, Z+					; get ddr
	;
	lpm		r25, Z					; get p
	;
	ret
