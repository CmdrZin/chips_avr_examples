/*
 * I2C Master Module
 *
 * org: 6/22/2014
 * rev: 10/2/2014
 * auth: Nels "Chip" Pearson
 *
 * Targets: ATmega164P, ATmega328P
 *
 * Usage:
 * 	.include packet_support.asm
 *
 * Requires:
 *	i2c_master.asm
 *	utilities.asm
 *
 */ 


/*
 * int i2c_write_pkt( SlaveAdrs, *buffer )
 * return: 0:ok, 1:if BUSY, 2:ERROR
 *
 * Buffer data: Hdr(Type+Nbytes), Data, ..., Blank (for CkSum)
 * Nbytes is for data only.
 * CkSum is calculated and added to the end of the buffer.
 *
 * input reg:	R17 - Slave Address
 *				X - buffer (includes space for CkSum after data)
 *
 * output reg: 	R17 - results of call (not of message)
 *
 * resources:	R16, X
 *
 */
i2c_write_pkt:
	ld		R18, X				; get header
	andi	R18, 0x0F			; mask out packet bytes.
	inc		R18					; add 1 for Header
	inc		R18					; add one for CkSum
;
	dec		R18					; adj back to data count only.
	call	util_gen_cksum		; Calculate CKSUM = (CKSUM ^ Data) ROL 1..Includes Header
								; index to end of buffer.
; point to end of buffer
	mov		YH, XH
	mov		YL, XL
	add		YL, R18
	clr		R16
	adc		YH, R16
	st		Y, R17				; store CKSUM at end of buffer.
;
	rjmp	i2c_write

/*
 * int i2c_read_pkt( SlaveAdrs, *buffer, maxByteCount )
 * return: 0:ok, 1:if BUSY, 2:ERROR
 *
 * Checks the buffer for a valid packet.
 *
 * Buffer data: Hdr(Type+Nbytes), Data, ..., CkSum
 * Nbytes is for data only.
 *
 * input reg:	R17 - Slave Address
 *				X - buffer (includes space for CkSum after data)
 *				R18 - max byteCount expected
 *
 * output reg: 	R17 - results of call (not of message)
 *
 * resources:	R16, X
 *
 */
i2c_read_pkt:
	ld		R18, X				; get header
	andi	R18, 0x0F			; mask out packet bytes.
	inc		R18					; add 1 for Header
	inc		R18					; add one for CkSum
;
	dec		R18					; adj back to data count only.
	call	util_gen_cksum		; Calculate CKSUM = (CKSUM ^ Data) ROL 1..Includes Header
								; index to end of buffer.
; point to end of buffer
	mov		YH, XH
	mov		YL, XL
	add		YL, R18
	clr		R16
	adc		YH, R16
	ld		R16, Y				; get CKSUM from end of buffer.
;
	cp		R16, R17
	breq	irp_skip00
; set CkSum error bit and Error state.
	ldi		R17, I2C_COMM_ERROR
	sts		i2c_comm_state, R17
; set ERROR flag
; TODO
	ret
;
irp_skip00:
; Valid
	clr		R17
	ret

