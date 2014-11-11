/*
 * I2C Slave Interface
 *
 * org: 6/22/2014
 * rev: 10/03/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include i2c_slave.asm
 *
 */ 

.equ	LCD_SLAVE_ADRS	= (0x25)
.equ	TONE_SLAVE_ADRS	= (0x57)


// TWI Slave Receiver staus codes
.equ	TWI_SRX_ADR_ACK				= 0x60  // Own SLA+W has been received ACK has been returned
.equ	TWI_SRX_ADR_ACK_M_ARB_LOST	= 0x68  // Own SLA+W has been received; ACK has been returned
.equ	TWI_SRX_GEN_ACK				= 0x70  // General call address has been received; ACK has been returned
.equ	TWI_SRX_GEN_ACK_M_ARB_LOST	= 0x78  // General call address has been received; ACK has been returned
.equ	TWI_SRX_ADR_DATA_ACK		= 0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
.equ	TWI_SRX_ADR_DATA_NACK		= 0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
.equ	TWI_SRX_GEN_DATA_ACK		= 0x90  // Previously addressed with general call; data has been received; ACK has been returned
.equ	TWI_SRX_GEN_DATA_NACK		= 0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
.equ	TWI_SRX_STOP_RESTART		= 0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave
// TWI Slave Transmitter staus codes
.equ	TWI_STX_ADR_ACK				= 0xA8  // Own SLA+R has been received; ACK has been returned
.equ	TWI_STX_ADR_ACK_M_ARB_LOST	= 0xB0  // Own SLA+R has been received; ACK has been returned
.equ	TWI_STX_DATA_ACK			= 0xB8  // Data byte in TWDR has been transmitted; ACK has been received
.equ	TWI_STX_DATA_NACK			= 0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
.equ	TWI_STX_DATA_ACK_LAST_BYTE	= 0xC8  // Last byte in TWDR has been transmitted (TWEA = 0); ACK has been received
// TWI Miscellaneous status codes
.equ	TWI_NO_STATE				= 0xF8  // No relevant state information available; TWINT = 0
.equ	TWI_BUS_ERROR				= 0x00  // Bus error due to an illegal START or STOP condition


.equ	I2C_ERROR_OV			= 0x81		// Overflow on input buffer.

.equ	I2C_BUFFER_SIZE			= 18

.equ	I2C_STATUS_LAST			= 0x01		// Last data byte received with NACK.
.equ	I2C_STATUS_NEW			= 0x02		// New data in input buffer.


.DSEG
// The index is added to the buffer base to locate or add data. Test cnt => size.
i2c_buffer_in_cnt:		.BYTE	1
i2c_buffer_in:			.BYTE	I2C_BUFFER_SIZE
in_XL:					.BYTE	1
in_XH:					.BYTE	1

i2c_buffer_out_cnt:		.BYTE	1
i2c_buffer_out:			.BYTE	I2C_BUFFER_SIZE
out_XL:					.BYTE	1
out_XH:					.BYTE	1
i2c_xmit_cnt:			.BYTE	1			; bytes sent so far.

i2c_error:				.BYTE	1			; Error code

i2c_status:				.BYTE	1			; flags

// Resume coding
.CSEG
/*
 * Initialize the TWI to support I2C Slave interface.
 * input reg:	none
 * output reg:	none
 *
 * I2C rate = CPU/(16 + 2(TWBR)*(4^TWPS0:1)) = 20^6Hz / 200 = 100kHz
 * 20Mhz / 200 = 100kHz .. 16 + (2*92)*1) = 200
 */
i2c_init_slave:
	ldi		R16, 92
	sts		TWBR, R16
;
	ldi		R16, 0			; Prescale TWPS1:0 = 00 .. 4^0 = 1
	sts		TWSR, R16
;
	ldi		R16, (TONE_SLAVE_ADRS<<1)|0x01	; allow general 00 adrs
	sts		TWAR, R16					; set Slave address
	clr		R16
	sts		TWAMR, R16					; enforce address compare..1's disable bit check.
;
	ldi		R16, (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)
	sts		TWCR, R16
;
	clr		R16							; clear flags
	sts		i2c_status, R16
;
	ret

/*
 * Initialize the TWI for SLAVE I2C interface.
 * input reg:	none
 * output reg:	none
 */
i2c_slave_init:
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)
	sts		TWCR, R16
	ret

/*
 * I2C interrupt service
 * 
 * input reg:
 * output reg:
 * resources: i2c_buff - data buffer
 *
 * Clear TWCR.TWINT by setting it to 1.
 *
 * Receiving Data FROM MASTER
 *	TWI_SRX_ADR_ACK				Own SLA+W has been received; ACK has been returned
 *	TWI_SRX_ADR_ACK_M_ARB_LOST	Own SLA+W has been received; ACK has been returned
 * 	TWI_SRX_GEN_ACK				General call address has been received; ACK has been returned
 *	TWI_SRX_GEN_ACK_M_ARB_LOST	General call address has been received; ACK has been returned
 *	TWI_SRX_ADR_DATA_ACK		Previously addressed with own SLA+W; data received; ACK has been returned
 *	TWI_SRX_ADR_DATA_NACK		Previously addressed with own SLA+W; data received; NACK has been returned as last byte?
 *	TWI_SRX_GEN_DATA_ACK		Previously addressed with general call; data has been received; ACK has been returned
 *	TWI_SRX_GEN_DATA_NACK		Previously addressed with general call; data has been received; NOT ACK has been returned
 *	TWI_SRX_STOP_RESTART		A STOP condition or repeated START condition has been received while still addressed as Slave
 *
 * Sending Data TO MASTER
 *	TWI_STX_ADR_ACK				Own SLA+R has been received; ACK has been returned
 *	TWI_STX_ADR_ACK_M_ARB_LOST	Own SLA+R has been received; ACK has been returned
 *	TWI_STX_DATA_ACK			Data byte in TWDR has been transmitted; ACK has been received
 *	TWI_STX_DATA_NACK			Data byte in TWDR has been transmitted; NACK has been received as last byte.
 *	TWI_STX_DATA_ACK_LAST_BYTE	Last byte in TWDR has been transmitted (TWEA = 0); ACK has been received
 *
 * When a SLA+W is received, the input buffer pointer is reset to the top of the buffer,
 * the byte count is reset to 0, and the flags are cleared.
 * If data is receivced after a SLA+W, it is placed into to input buffer and the input count
 * incremented provided that the current input count is less than the maximum buffer size.
 * Last data sent will have a NACK from the Master to signal the last byte. It is also placed
 * into the buffer if there is room.
 * A STOP resets the interface.
 *
 * When a SLA+R is recieved, the input buffer pointer is reset to the top of the buffer and
 * the byte count is reset to 0. The first byte is placed into the data register to be sent.
 * For each byte sent, load next byte. If the buffer until buffer is empty, resend the last
 * byte.
 *
 */
ii_skip80e:
	rjmp	ii_skip80
ii_skip88e:
	rjmp	ii_skip88
ii_skip90e:
	rjmp	ii_skip90
ii_skip98e:
	rjmp	ii_skip98
ii_skipA0e:
	rjmp	ii_skipA0
ii_skipA8e:
	rjmp	ii_skipA8
ii_skipB0e:
	rjmp	ii_skipB0
ii_skipB8e:
	rjmp	ii_skipB8
ii_skipC0e:
	rjmp	ii_skipC0
ii_skipC8e:
	rjmp	ii_skipC8

i2c_intr:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	R16
	push	R17
;
	lds		R16, TWSR				; get status
	andi	R16, 0xF8				; mask out bits
; Receive data from Master
	cpi		R16, TWI_SRX_ADR_ACK
	breq	ii_skip60
	cpi		R16, TWI_SRX_ADR_ACK_M_ARB_LOST
	breq	ii_skip68
	cpi		R16, TWI_SRX_GEN_ACK
	breq	ii_skip70
	cpi		R16, TWI_SRX_GEN_ACK_M_ARB_LOST
	breq	ii_skip78
	cpi		R16, TWI_SRX_ADR_DATA_ACK
	breq	ii_skip80e
	cpi		R16, TWI_SRX_ADR_DATA_NACK
	breq	ii_skip88e
	cpi		R16, TWI_SRX_GEN_DATA_ACK
	breq	ii_skip90e
	cpi		R16, TWI_SRX_GEN_DATA_NACK
	breq	ii_skip98e
	cpi		R16, TWI_SRX_STOP_RESTART
	breq	ii_skipA0e
; Send data to Master
	cpi		R16, TWI_STX_ADR_ACK
	breq	ii_skipA8e
	cpi		R16, TWI_STX_ADR_ACK_M_ARB_LOST
	breq	ii_skipB0e
	cpi		R16, TWI_STX_DATA_ACK
	breq	ii_skipB8e
	cpi		R16, TWI_STX_DATA_NACK
	breq	ii_skipC0e
	cpi		R16, TWI_STX_DATA_ACK_LAST_BYTE
	breq	ii_skipC8e
;
	rjmp	ii_reset_intr			; Unknown Status byte.
;
ii_skip60:
; Own SLA+W has been received; ACK has been returned. Expect to receive data.
	ldi		R16, LOW(i2c_buffer_in)
	sts		in_XL, R16
	ldi		R16, HIGH(i2c_buffer_in)
	sts		in_XH, R16
	clr		R16
	sts		i2c_buffer_in_cnt, R16		; track number of bytes received.
	sts		i2c_status, R16				; clear flags	
	rjmp	ii_reset_intr
;
ii_skip68:
	rjmp	ii_reset_intr
;
ii_skip70:
; General call address has been received; ACK has been returned
; A General 00 address has been received..IGNORE
ii_skip78:
; not Supported
	rjmp	ii_reset_intr

ii_skip80:
; Previously addressed with own SLA+W; data received; ACK has been returned
; If last byte, then mark buffer as new message.
	lds		R16, i2c_status
	cpi		R16, I2C_STATUS_LAST
	breq	ii_skip82
	ldi		R16, I2C_STATUS_NEW			; New message
	sts		i2c_status, R16
;
ii_skip82:
	lds		R17, TWDR			; get data
; check for overflow
	lds		R16, i2c_buffer_in_cnt
	cpi		R16, I2C_BUFFER_SIZE
	brlt	ii_skip81
; no..error OV
	ldi		R16, I2C_ERROR_OV
	sts		i2c_error, R16
	rjmp	ii_reset_intr				; EXIT
;
ii_skip81:
; Save data to i2c_buffer_in
	push	XL
	push	XH
;
	inc		R16							; add 1 to count
	sts		i2c_buffer_in_cnt, R16
;
	lds		XL, in_XL
	lds		XH, in_XH
	st		X+, R17						; save data to buffer
	sts		in_XL, XL
	sts		in_XH, XH
;
	pop		XH
	pop		XL
; TWI Interface enabled, Enable TWI Interupt and clear the flag to send byte, Send ACK after next reception
	rjmp	ii_reset_intr
;
ii_skip88:
; Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
; Slave no longer addressed.
	ldi		R16, I2C_STATUS_LAST		; flag as last byte
	sts		i2c_status, R16
	rjmp	ii_skip80					; save data
;
ii_skip90:
; General address data has been received; ACK has been returned
ii_skip98:
; not Supported
	rjmp	ii_reset_intr
;
ii_skipA0:
; A STOP condition or repeated START condition has been received while still addressed as Slave
; Enter not addressed mode and listen to address match, Enable TWI-interface and release TWI pins,
; Enable interrupt and clear the flag, Wait for new address match
	rjmp	ii_reset_intr
;
ii_skipA8:
; SLA+R has been received; ACK has been returned. Setup to send data.
	push	XL
	push	XH
;
	ldi		XL, LOW(i2c_buffer_out)
	ldi		XH, HIGH(i2c_buffer_out)
	ld		R16, X+
	sts		TWDR, R16							; load output data.
; TWI Interface enabled, enable TWI Interupt, and clear the flag to send byte
	sts		out_XL, XL
	sts		out_XH, XH
;
	ldi		R16, 1
	sts		i2c_xmit_cnt, R16					; track number of bytes sent.
;
	pop		XH
	pop		XL
;
	rjmp	ii_reset_intr
;
ii_skipB0:
	rjmp	ii_skipA8

ii_skipB8:
;Data byte in TWDR has been transmitted; ACK has been received. No STOP, so send next byte.
; check for data
	lds		R16, i2c_xmit_cnt
	lds		R17, i2c_buffer_out_cnt
	cp		R16, R17
	brge	ii_skipB9							; data available?..no, sent last byte.
;yes
	inc		R16
	sts		i2c_xmit_cnt, R16					; update the number of bytes sent.
;
	push	XL
	push	XH
;
	lds		XL, out_XL
	lds		XH, out_XH
	ld		R16, X+
	sts		TWDR, R16							; load output data.
; TWI Interface enabled, enable TWI Interupt, and clear the flag to send byte
	sts		out_XL, XL
	sts		out_XH, XH
;
	pop		XH
	pop		XL
; data sent.
ii_skipB9:
; TWI Interface enabled, enable TWI Interupt, and clear the flag to send byte
	rjmp	ii_reset_intr
;
ii_skipC0:
; Data byte in TWDR has been transmitted; NACK has been received as last byte.
ii_skipC8:
; last data byte sent..NACK -> End of expected data set. No more data expected in this request.
; Enable TWI-interface and release TWI pins, Keep interrupt enabled and clear the flag, Answer next address match
	rjmp	ii_reset_intr
;
ii_reset_intr:
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)
	sts		TWCR, R16
;
ii_exit:
	pop		R17
	pop		R16
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti
