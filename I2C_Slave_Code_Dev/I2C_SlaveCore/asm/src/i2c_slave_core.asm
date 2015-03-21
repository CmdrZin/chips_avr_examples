/*
 * I2C Slave Core Service Code
 *
 * org: 02/23/2015
 * rev: 03/17/2015
 * auth: Nels "Chip" Pearson
 *
 * This code forms the Core Command Processor (CCP) of a Slave Board.
 * It will pass any non-Core command onto the Board Command Processor of the Target.
 *
 * Target: Any TWI AVR Processor. This: I2C Demo board, 20MHz, ATmega164P
 *
 *
 * Dependentcies
 *	i2c_slave2.asm
 *	i2c_slave_i2cDemo.asm
 *
 */



// I2C COMMANDS - Each Service .inc file is customized for the board it supports.
.equ	I2CS_FIRST_BOARD_CMD	= 0x80		; test point to branch to board service.

.DSEG



.CSEG
// Call hardware inits from here also
slave_core_init:
	call	i2c_slave_start
;
	ret

/*
 * Command Processor. Call from main().
 *
 * Core Slave Commands:		0x00 - 0x7F
 * Board Specific Commands:	0x80 - 0xFF
 *
 * If WRITE, CMD and data are in i2c_in_buff. Service CMD write.
 * If READ, CMD is in i2c_in_buff. Place CMD + Data[] into i2c_out_buff.
 *
 */
slave_core_service:
	lds		r16, i2c_status
	cpi		r16, I2C_STATUS_MSG_RCVD
	breq	scs_skip00
	rjmp	scs_exit
; Service Message
scs_skip00:
; Clear flag
	ldi		r16, I2C_STATUS_IDLE
	sts		i2c_status, r16
; Get COMMAND
	ldi		XL, low(i2c_buffer_in)
	ldi		XH, high(i2c_buffer_in)
	ldi		r19, I2C_BUFFER_SIZE
	call	fifo_get				; XL, XH, r19 preserved.
	tst		r18						; valid data?
	breq	scs_skip01
	rjmp	scs_exit
scs_skip01:
	mov		r16, r17
; select processor
	cpi		r16, I2CS_FIRST_BOARD_CMD
	brsh	scs_skip02				; unsigned compare
// Core Command
	call	core_command_processor
	rjmp	scs_exit
;
scs_skip02:
// Board Command
	call	board_command_processor		; --> i2c_slave_i2cDemo.asm
	rjmp	scs_exit
;
scs_exit:
;
// Restart Slave I2C interface here? see TODO. May have a configuration flag.
;
	ret


// CORE COMMANDS ( LOW() matches in_data[0] )
.equ	CORE_GET_MFG_ID		= 0x0400		;  4 bytes for CMD 0x00
.equ	CORE_SET_ADRS		= 0x0401		;  4 bytes for CMD 0x01
.equ	CORE_GET_ID			= 0x0C02		; 12 bytes for CMD 0x02
.equ	CORE_GET_MFG_NAME	= 0x1003		; 16 bytes for CMD 0x03
.equ	CORE_GET_DEV_NAME	= 0x0004
.equ	CORE_GET_PARAM_NUM	= 0x0005
.equ	CORE_GET_PARAM_PAIR	= 0x0006


/*
 * Core Command Processor. Call from slave_service().
 *
 * Core Slave Commands:		0x00 - 0x7F
 *
 * If WRITE, CMD and data are in i2c_in_buff. Service CMD write.
 * If READ, CMD is in i2c_in_buff. Place CMD + Data[] into i2c_out_buff.
 *
 * input:	r17		command
 *			X		i2c_buffer_in
 *			r19		buffer size
 *
 * output:	none
 *
 */
core_command_processor:
// Put SET commands here.
	cpi		r17, LOW(CORE_SET_ADRS)
	brne	ccp_skip000
// TODO: Read buffer and check sequence to set address.
	rjmp	ccp_exit
;
// Service GET commands
ccp_skip000:
	cpi		r17, LOW(CORE_GET_MFG_ID)
	brne	ccp_skip010
; Copy Manufactures ID into output buffer.
	ldi		ZL, LOW(i2c_manufacturer_id<<1)
	ldi		ZH, HIGH(i2c_manufacturer_id<<1)
	ldi		r20, HIGH(CORE_GET_MFG_ID)				; 4 bytes
	rjmp	ccp_text_exit
;
ccp_skip010:
	cpi		r17, LOW(CORE_GET_ID)
	brne	ccp_skip020
; Copy board ID into output buffer.
; TODO: Replace this with ID data stored in EEPROM
	ldi		ZL, LOW(i2c_board_id<<1)
	ldi		ZH, HIGH(i2c_board_id<<1)
	ldi		r20, HIGH(CORE_GET_ID)					; 12 bytes
	rjmp	ccp_text_exit
;
ccp_skip020:
	cpi		r17, LOW(CORE_GET_MFG_NAME)
	brne	ccp_skip030
; Copy Manufactures ID into output buffer.
	ldi		ZL, LOW(i2c_manf_name<<1)
	ldi		ZH, HIGH(i2c_manf_name<<1)
	ldi		r20, HIGH(CORE_GET_MFG_NAME)			; 16 bytes
	rjmp	ccp_text_exit
;
ccp_skip030:
	cpi		r17, LOW(CORE_GET_DEV_NAME)
	cpi		r17, LOW(CORE_GET_PARAM_NUM)
	cpi		r17, LOW(CORE_GET_PARAM_PAIR)
	rjmp	ccp_exit
;
/* Set up Z = &text and r20 with text length. */
ccp_text_exit:
; Set up for output copy
	ldi		XL, low(i2c_buffer_out)
	ldi		XH, high(i2c_buffer_out)
	ldi		r19, I2C_BUFFER_SIZE
ccp_text_loop:					; do: while(r20)
	tst		r20
	breq	ccp_exit			; done. Next Read will return data in i2c_buffer_out.
; next byte
	lpm		r17, Z+				; get data from flash
	call	fifo_put
	dec		r20
	rjmp	ccp_text_loop
;
ccp_exit:
	ret

i2c_manufacturer_id:			; 4 char
.db		0x31,0x41,0x59,0x27
i2c_board_id:					; 12 char build id YYMMDD+REV[1]+'.'+Board[4]
.db		'1','5','0','3','2','0','A','.'
.db		'I','2','C','B'
i2c_manf_name:					; 16 char
.db		'A','L','T','R','U','I','S','T'
.db		'E','C','H',' ',' ',' ',' ',' '
