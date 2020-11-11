/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Nels D. "Chip" Pearson (aka CmdrZin)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
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
 *
 * utilShift.s
 *
 * Created: 11/10/2020 5:15:29 PM
 *  Author: Chip
 *
 * r0 general temp register
 * r1 = 0 always
 * r2-r17		protected (save/restore if used)
 * r18-r25		general use
 * X = r27:r26	general use
 * Y = r29:r28	protected (save/restore if used)
 * Z = r31:r30	general use - FLASH access
 *
 * Parameters: r25:r24, r23:r22, r21:r20, r19:r18
 *              b1  b0   b3  b2   b5  b4   b7  b6
 */ 

  #include <avr/io.h>

  .global shiftRight


/* Input:
 *   r25:r24 (H.L) = buffer ptr (uint8_t* buffer)
 *   r23:r22 (0,L) = size (uint8_t size)
 * Output:
 *   r25:r24 (0,L) = r24 has LSBit.
 *
 * Starting with buffer[size-1], shift each byte right by 1.
 * Return:
 *   The bit shifted out of the last byte buffer[0]>>1.
 * Use r27:r26 for pointer
 * Use r25:r18 as temp registers.
 * r1 = 0 always.
 */
shiftRight:
	tst		r22			; check for zero
	breq	sR_zero_exit
; ok to continue. size > 0.
	adc		r24,r22		; add size to buffer base.
	adc		r25,r1		; propigate carry
	movw	r26,r24		; set X pointer = buffer[size]
	movw	r30,r24		; set Z pointer = buffer[size]
; X pointer -> last byte in buffer.
	clc					; clear CY bit
sR_loop:
	ld		r0,-X		; predecriment and load
	ror		r0			; shift right. LSbit -> CY
	st		-Z,r0		; update buffer
	dec		r22			; dec size. does not affect CY bit.
	brne	sR_loop		; repeat if not zero
; save the CY bit into r22
	mov		r24,r1		; set to 0
	adc		r24,r1		; bring in CY
	rjmp	sR_exit
;
sR_zero_exit:
	mov		r24,r1		; zero return
sR_exit:
	mov		r25,r1		; zero MSB
    ret
