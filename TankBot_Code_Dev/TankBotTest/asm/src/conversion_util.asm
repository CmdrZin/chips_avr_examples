/*
 * Conversion Utilities
 *
 * org: 10/21/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Any
 *
 * Usage:
 * 	.include conversion_util.asm
 *
 * These functions are for converting data formats
 *
 *
 */ 

.DSEG
con_util_bcd_buff:		.BYTE	2
con_bin_ascii_buff:		.BYTE	5

.CSEG


/*
 * bin8hex()
 *
 * Convert 8bit to HEX ascii and appends to buffer and update byte count.
 *
 * input:	R17 - data
 *			r18	- byte count
 *			X	- text buffer
 *
 * output:	*X -> HEX char
 *
 * Algorythm
 *   Generate two HEX char for 8 bit bin.
 *
 */
bin8hex:

	ret


/*
 * bin8dec()
 *
 * Convert 8bit to decimal ascii and appends to buffer and update byte count.
 *
 * input:	R17 - b7:0
 *			r18	- byte count
 *			X	- text buffer
 *
 * output:	*X -> BCD char
 *
 * Algorythm
 *   Generates 0 -> 255 BCD from binary with leading 0s removed.
 *   Test if any nibble of >=5 is to be shifted, then add 3 to nibble.
 *   X[1].X[0]<-r17
 *   r16 used as test buffer.
 *
 */
bin8dec:
	push	r16
	push	r17
	push	r19
	push	r20
	push	r21
	push	r22
	push	r23
	push	r24
; presets
	clr		r16			; 
	ldi		R19, 0x03
	ldi		R20, 0x30
	ldi		R21, 5		; shift count
; shift 2 bits
	lsl		R17
	rol		R16			; shift to test reg
	lsl		R17
	rol		R16			; shift to test reg
	mov		r23, r16
	clr		r24
; algorythm
b8d_loop00:
; Shift into buffer
	lsl		R17
	mov		r16, r23
	rol		R16
	mov		r23, R16
	mov		R16, r24
	rol		R16
	mov		r24, R16
; test >=0x05 >=0x50
	call	con_test_bcd
	mov		r23, r16
	mov		r16, r24
	call	con_test_bcd
	mov		r24, r16
;
	dec		R21
	brne	b8d_loop00		; do next bit
; do last shift
; Shift into buffer
	lsl		R18			; LSBs
	rol		R17
	mov		R16, r23
	rol		R16			; shift to test reg
	mov		r23, R16
	mov		R16, r24
	rol		R16			; shift to test reg
	mov		r24, R16
;
	pop		r24
	pop		r23
	pop		r22
	pop		r21
	pop		r20
	pop		r19
	pop		r18
	pop		r17
	pop		r16
;
	ret


/*
 * r16 = test reg data
 *
 */
con_test_bcd:
	push	r19
	push	r20
;
	ldi		R19, 0x03			; preloaded adjustments
	ldi		R20, 0x30
; test >=0x05
	push	R16
	andi	R16, 0x0F
	cpi		R16, 0x05
	brlo	ctb_skip00			; unsigned check
	pop		R16
	add		R16, R19			; add 0x03
	rjmp	ctb_skip01
;
ctb_skip00:
	pop		R16
;
ctb_skip01:
; test >=0x50
	push	R16
	andi	R16, 0xF0
	cpi		R16, 0x50
	brlo	ctb_skip03			; unsigned check
	pop		R16
	add		R16, R20			; add 0x30
	rjmp	ctb_skip04
;
ctb_skip03:
	pop		R16
;
ctb_skip04:
	pop		r20
	pop		r19
;
	ret




/*
 * bin2bcd_10_4()
 *
 * Convert 10 bits to 4 digit BCD
 *
 * input:	R17 - b9:2
 *			R18 - b1:0
 *
 * output:	con_util_bcd_buff -> BCD..LSB
 *
 * Algorythm
 *   Test if any nibble of >=5 is to be shifted, then add 3 to nibble.
 *
 */
bin2bcd_10_4:
	push	r16
	push	r17
	push	r18
	push	r19
	push	r20
	push	r21
; presets
	ldi		R19, 0x03
	ldi		R20, 0x30
	ldi		R21, 7		; shift count
; shift 2 bits
	lsl		R18			; LSBs
	rol		R17
	rol		R16			; shift to test reg
	lsl		R18			; LSBs
	rol		R17
	rol		R16			; shift to test reg
	sts		con_util_bcd_buff, R16
; algorythm
bb10_4_loop00:
; Shift into buffer
	lsl		R18			; LSBs
	rol		R17
bb10_4_loop01:
	lds		R16, con_util_bcd_buff
	rol		R16
	sts		con_util_bcd_buff, R16
	lds		R16, con_util_bcd_buff+1
	rol		R16
	sts		con_util_bcd_buff+1, R16
;
	ldi		XL, low(con_util_bcd_buff)
	ldi		XH, high(con_util_bcd_buff)
	ldi		R22, 2		; buffer size
bb10_4_loop02:
; test >=0x05
	ld		R16, X
	push	R16
	andi	R16, 0x0F
	cpi		R16, 0x05
	brlo	bb10_4_skip00		; unsigned check
	pop		R16
	add		R16, R19			; add 0x03
	rjmp	bb10_4_skip01
;
bb10_4_skip00:
	pop		R16
bb10_4_skip01:
; test >=0x50
	push	R16
	andi	R16, 0xF0
	cpi		R16, 0x50
	brlo	bb10_4_skip03		; unsigned check
	pop		R16
	add		R16, R20			; add 0x30
	rjmp	bb10_4_skip04
;
bb10_4_skip03:
	pop		R16
bb10_4_skip04:
	st		X+, R16
	dec		R22
	brne	bb10_4_loop02		; test next buffer byte
;
	dec		R21
	brne	bb10_4_loop00		; do next bit
; do last shift
; Shift into buffer
	lsl		R18			; LSBs
	rol		R17
	lds		R16, con_util_bcd_buff
	rol		R16			; shift to test reg
	sts		con_util_bcd_buff, R16
	lds		R16, con_util_bcd_buff+1
	rol		R16			; shift to test reg
	sts		con_util_bcd_buff+1, R16
;
	pop		r21
	pop		r20
	pop		r19
	pop		r18
	pop		r17
	pop		r16
;
	ret

/*
 * Convert 10bit ADC to HEX
 * Right shift Left shifted ADC value.
 * 
 * input:	R17 - b9:2
 *			R18 - b1:0
 *
 * output:	X -> ASCII..MSB
 *
 * NOTE: Generates NULL terminated string.
 */
con_adcLS_ascii:
	push	r16
	push	r17
	push	r18
;
	ldi		R16, 6
; Shift right to right justify ADC data
caa_loop00:
	lsr		R17
	ror		R18
	dec		R16
	brne	caa_loop00
;
con_bin_ascii:
; Convert and load up buffer
	ldi		XL, low(con_bin_ascii_buff)
	ldi		XH, high(con_bin_ascii_buff)
;
	mov		R16, R17
	andi	R16, 0xF0
	swap	R16
	call	con_4bit2hex
	st		X+, R16
;
	mov		R16, R17
	andi	R16, 0x0F
	call	con_4bit2hex
	st		X+, R16
;
	mov		R16, R18
	andi	R16, 0xF0
	swap	R16
	call	con_4bit2hex
	st		X+, R16
;
	mov		R16, R18
	andi	R16, 0x0F
	call	con_4bit2hex
	st		X+, R16
;
	clr		R16				; add NULL
	st		X, R16
;
	ldi		XL, low(con_bin_ascii_buff)
	ldi		XH, high(con_bin_ascii_buff)
;
	pop		r18
	pop		r17
	pop		r16
;
	ret

/*
 * Internal function call
 * Safe call
 *
 * input:	R16 - Data 0-F
 * output:	R16 - ASCII 0-9,A-F
 *
 */
con_4bit2hex:
	push	R19
	push	R20
;
	ldi		R19, '0'
	ldi		R20, 'A'-10
; test
	cpi		R16, 10
	brlo	c4_skip00
; A-F
	add		R16, R20
	rjmp	c4_exit
; 0-9
c4_skip00:
	add		R16, R19
;
c4_exit:
	pop		R20
	pop		R19
	ret
