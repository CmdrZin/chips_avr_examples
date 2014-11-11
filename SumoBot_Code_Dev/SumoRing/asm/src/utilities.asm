/*
 * Utilities for I2C Master Module
 *
 * org: 6/22/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include utilities.asm
 *
 */ 


/*
 * Generate CKSUM
 *
 * CKSUM = (CKSUM ^ Data) ROL 1
 * 
 * input:	X(r27:26) reg -> Data buffer
 *			R18: Number of bytes
 *
 * output:	R17: CKSUM
 *
 * Registers: All saved and restored
 *
 */
util_gen_cksum:
	push	R16
	push	R17
	push	R18
	push	XL
	push	XH
;
	clr		R17
ugc_loop00:
	ld		R16, X+
	eor		R17, R16
	lsl		R17
	brcc	ugc_skip00
	ori		R17, 0x01				; bring in CY bit
;
ugc_skip00:
	dec		R18
	brne	ugc_loop00				; done?
; yes
	pop		XH
	pop		XL
	pop		R18
	pop		R17
	pop		R16
;
	ret
