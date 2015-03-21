/*
 * I2C Slave Service Code - I2C Demo Board
 *
 * org: 02/23/2015
 * rev: 03/17/2015
 * auth: Nels "Chip" Pearson
 *
 * This code forms the Board Command Processor (BCP) of a Slave Board.
 *
 * Target: Any TWI AVR Processor. I2C Demo Board, 20MHz, ATmega164P
 *
 *
 * Dependentcies
 *	io control and keypad
 *
 */

// I2C COMMANDS - Each Service file is customized for the board it supports.
// These command will be in a header for the I2C Master to use.
.equ	BRD_READ_BUTTON		= 0x0183	; out[0] = 1=Pressed..0=Released

.equ	BRD_READ_KEYPAD		= 0x0184	; out[0] = last valid key [0..F]


// BOARD COMMANDS ( in[0] = CMD )
.equ	BRD_SET_DISPLAY		= 0x0181	; in[1] = Left 7-seg LED
										; in[2] = Right 7-seg LED

.equ	BRD_SET_LED			= 0x0182	; in[1] = 0=Off..1=On


/*
 * Board Command Processor. Call from slave_core_service().
 *
 * Board Specific Commands:	0x80 - 0xFF
 *
 * If WRITE, CMD and data are in i2c_in_buff. Service CMD write.
 * If READ, CMD is in i2c_in_buff. Place Data[] into i2c_out_buff.
 *
 * input:	r17		command
 *			X		i2c_buffer_in
 *			r19		buffer size
 *
 * output:	none
 *
 */
board_command_processor:
	cpi		r17, LOW(BRD_SET_LED)
	brne	bcp_skip010
; Set LED
	call	fifo_get
	tst		r18					; data?
	breq	bcp_skip001
	rjmp	bcp_exit			; no..EXIT
; yes
bcp_skip001:
	tst		r17
	breq	bcp_skip002
; ON
	call	turn_on_led			; Turn LED ON
	rjmp	bcp_exit
;
bcp_skip002:
; OFF
	call	turn_off_led		; Turn LED OFF
	rjmp	bcp_exit
;
bcp_skip010:
	cpi		r17, LOW(BRD_READ_BUTTON)
	brne	bcp_skip020
; TODO: Test Button. Put state into out buffer FIFO
	rjmp	bcp_exit
;
bcp_skip020:
	cpi		r17, LOW(BRD_READ_KEYPAD)
	brne	bcp_skip030
; TODO: Put last valid key press into out buffer FIFO
	rjmp	bcp_exit
;
bcp_skip030:
	cpi		r16, LOW(BRD_SET_DISPLAY)
	brne	bcp_exit
; Set display to data byte.
	call	fifo_get
	tst		r18					; data?
	breq	bcp_skip031
	rjmp	bcp_exit			; no..EXIT
; yes
bcp_skip031:
	push	r17
;
	andi	r17, 0x0F			; LSBs
	call	dsp_dual_store
	pop		r17
	swap	r17
	andi	r17, 0x0F			; MSBs
	call	dsp_dual_store
;
	rjmp	bcp_exit
;
bcp_exit:
	ret
