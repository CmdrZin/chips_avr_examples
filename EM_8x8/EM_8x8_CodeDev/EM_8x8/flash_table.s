/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * flash_table.s
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 * revision: 01/13/2016	0.02	ndp		Add copy table to SRAM.
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

.global flash_copy8
/*
 * r25:24	= index (16 bit to allow full x8 multiply)
 * r23:22	= table
 * r21:20	= SRAM address
 *
 * table has 8 bytes per entry. An array of uint8_t values (see sysdefs.h ICON_DATA)
 * This function copies 8 bytes into the SRAM address.
 * returns void
 *
 */
 flash_copy8:
	; multiply 16-bit index by 8
	lsl 	r24			; x2
	rol		r25
	lsl 	r24			; x4
	rol		r25
	lsl 	r24			; x8
	rol		r25
	; Z = table
	mov		r30, r22
	mov		r31, r23
	; add index
	add		r30, r24
	adc		r31, r25
	; X = SRAM
	mov		r26, r20
	mov		r27, r21
	; move data
	ldi		r25, 8
fc8_loop0:
	lpm		r24, Z+
	st		X+, r24
	dec		r25
	brne	fc8_loop0
	;
	ret
