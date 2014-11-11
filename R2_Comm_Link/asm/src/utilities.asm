/*
 * I2C Master Module
 *
 * org: 6/22/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: I2C Demo Board w/ display and keypad I/O, 20MHz
 *
 * Usage:
 * 	.include i2c_master.asm
 *
 */ 


/*
 * Calculate CKSUM
 *
 * CKSUM = (CKSUM ^ Data) ROL 1
 * 
 * input:	X reg -> Data buffer
 *			R18: Number of bytes
 * output:	R17: CKSUM
 *			X: -> CKSUM
 *
 */
util_cksum:
	clr		R17
uc_loop00:
	ld		R16, X+
	eor		R17, R16
	lsl		R17
	brcc	uc_skip00
	ori		R17, 0x01				; bring in CY bit
;
uc_skip00:
	dec		R18
	brne	uc_loop00				; done?
; yes
	ret
