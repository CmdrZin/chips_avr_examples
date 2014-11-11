/*
 * I2C Slave Interface
 *
 * org: 6/22/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include i2c_slave.asm
 *
 */ 

.equ	SLAVE_ADRS	= (0x57)


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

// I2C Flags
.equ	I2CF_IN_MSG				= 0x01		; if set=1, New I2C Message in I2C IN buffer
											; Be sure to clear if another message starts.


.DSEG
// The index is added to the buffer base to locate or add data. Test cnt => size.
i2c_buffer_in_cnt:		.BYTE	1
i2c_buffer_in:			.BYTE	I2C_BUFFER_SIZE
in_XL:					.BYTE	1
in_XH:					.BYTE	1
i2c_recv_cnt:			.BYTE	1			; bytes received so far.

i2c_buffer_out_cnt:		.BYTE	1
i2c_buffer_out:			.BYTE	I2C_BUFFER_SIZE
out_XL:					.BYTE	1
out_XH:					.BYTE	1
i2c_xmit_cnt:			.BYTE	1			; bytes sent so far.

i2c_error:				.BYTE	1			; Error code

i2c_flags:				.BYTE	1			; I2C Flags

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
	ldi		R16, (SLAVE_ADRS<<1)|0x01	; allow general 00 adrs
	sts		TWAR, R16					; set Slave address
	clr		R16
	sts		TWAMR, R16					; enforce address compare..1's disable bit check.
;
	ldi		R16, (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)
	sts		TWCR, R16
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
 * Get Data from Buffer In
 * input reg:	none
 * output reg:	R17: Data
 *	`			R18:	0: valid..1:no data
 */
i2c_read_buffer_in:
	lds		R16, i2c_buffer_in_cnt
	tst		R16
	brne	irbi_skip00
; no data
	ldi		R18, 1
	ret
;
irbi_skip00:
	dec		R16							; adj count
	sts		i2c_buffer_in_cnt, R16		; update
;
	ldi		XL, LOW(i2c_buffer_in)
	ldi		XH, HIGH(i2c_buffer_in)
	ld		R17, X
	clr		R18							; valid data
;
	ret

/*
 * Write Data to Buffer Out
 * input reg:	R17
 * output reg:	none
 *
 */
i2c_write_buffer_out:
	lds		R16, i2c_buffer_out_cnt
	cpi		R16, I2C_BUFFER_SIZE
	brlt	iwbo_skip00
; no room..error???
	ret
;
iwbo_skip00:
	inc		R16							; adj count
	sts		i2c_buffer_out_cnt, R16		; update
;
	ldi		XL, LOW(i2c_buffer_out)
	ldi		XH, HIGH(i2c_buffer_out)
	ld		R17, X
;
	ret

/*
 * Clear error flag
 * input reg:	none
 * output reg:	none
 */
 i2c_clear_error:
	clr		R16
	sts		i2c_error, R16
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
 * Sending Data TO MASTER
 *	TWI_STX_ADR_ACK			Own SLA+R has been received; ACK has been returned
 *							Never return NACK. Hold off not used.
 *	TWI_STX_DATA_ACK		Data byte in TWDR has been transmitted; ACK has been received
 *	TWI_STX_DATA_NACK		Data byte in TWDR has been transmitted; NACK has been received as last byte.
 *
 * Receiving Data FROM MASTER
 *	TWI_SRX_ADR_ACK			Own SLA+W has been received ACK has been returned
 *							Never return NACK
 *	TWI_SRX_ADR_DATA_ACK	Previously addressed with own SLA+W; data received; ACK has been returned
 *	TWI_SRX_ADR_DATA_NACK	Previously addressed with own SLA+W; data received; NACK has been returned as last byte?
 *
 *	TWI_SRX_STOP_RESTART	A STOP condition or repeated START condition has been received while still addressed as Slave
 *
 *	if TWSR = TWI_STX_ADR_ACK, then ok
 *	if TWSR = TWI_STX_DATA_ACK, then copy TWDR data into i2c_buffer_in
 *
 *	if TWSR = TWI_SRX_ADR_ACK, then copy i2c_buffer_out TWDR
 *	if TWSR = TWI_SRX_ADR_DATA_ACK, then ok, copy next byte from buffer
 *
 *	else ignore for now.
 */
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
;
	cpi		R16, TWI_STX_ADR_ACK
	breq	ii_skip00
	cpi		R16, TWI_STX_DATA_ACK
	breq	ii_skip10
	cpi		R16, TWI_STX_DATA_NACK
	breq	ii_skip20
	cpi		R16, TWI_SRX_ADR_ACK
	breq	ii_skip30
	cpi		R16, TWI_SRX_ADR_DATA_ACK
	brne	ii_skip940
	rjmp	ii_skip40
ii_skip940:
	cpi		R16, TWI_SRX_STOP_RESTART
	brne	ii_skip950
	rjmp	ii_skip50
ii_skip950:
	cpi		R16, TWI_SRX_GEN_ACK
	brne	ii_skip960
	rjmp	ii_skip60
ii_skip960:
	rjmp	ii_skip50
	rjmp	ii_reset_intr						; Status byte not supported yet.
;
ii_skip00:
; SLA+R has been received; ACK has been returned. Start sending data.
; TEST ++
	ldi		R16, 0xCC
	sts		led_bank, R16
; TEST --
; Adrs+Read..set up data from top of buffer..
; NOTE: Always has min 2 bytes. Header (type,size) + Data,Data,... + CKSUM. Data may not exist.
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
ii_skip10:
;Data byte in TWDR has been transmitted; ACK has been received. No STOP, so send next byte.
; DEBUG ++
	ldi		R16, 0x02
	sts		led_cntl, R16
; check for data
	lds		R16, i2c_xmit_cnt
	lds		R17, i2c_buffer_out_cnt
	cp		R16, R17
	brge	ii_skip11							; data available?
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
ii_skip11:
; TWI Interface enabled, enable TWI Interupt, and clear the flag to send byte
	rjmp	ii_reset_intr
;
ii_skip20:
; Data byte in TWDR has been transmitted; NACK has been received as last byte.
; last data byte sent..NACK -> End of expected data set. No more data expected in this request.
; DEBUG ++
	ldi		R16, 0x03
	sts		led_cntl, R16
; Enable TWI-interface and release TWI pins, Keep interrupt enabled and clear the flag, Answer next address match
	rjmp	ii_reset_intr
;
ii_skip30:
; SLA+W has been received ACK has been returned. Expect to receive data.
	ldi		R16, LOW(i2c_buffer_in)
	sts		in_XL, R16
	ldi		R16, HIGH(i2c_buffer_in)
	sts		in_XH, R16
	clr		R16
	sts		i2c_buffer_in_cnt, R16				; track number of bytes received.
; DEBUG ++
	ldi		R16, 0x04
	sts		led_cntl, R16
; TWI Interface enabled, Enable TWI Interupt and clear the flag to send byte, Expect ACK on this transmission
	lds		R16, i2c_flags
	cbr		R16, I2CF_IN_MSG		; clear flag
	sts		i2c_flags, R16
;
	rjmp	ii_reset_intr
;
ii_skip40:
; Previously addressed with own SLA+W; data received; ACK has been returned
; DEBUG ++
	ldi		R16, 0x05
	sts		led_cntl, R16
; Data received..send I2C IN buffer
	lds		R17, TWDR			; get data and save to I2C IN buffer.
; check for overflow
	lds		R16, i2c_buffer_in_cnt
	cpi		R16, I2C_BUFFER_SIZE
	brlt	ii_skip41
; no..error OV
	ldi		R16, I2C_ERROR_OV
	sts		i2c_error, R16
	rjmp	ii_reset_intr				; EXIT
;
ii_skip41:
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
;                                 // 
ii_skip50:
; DEBUG ++
	ldi		R16, 0x06
	sts		led_cntl, R16
;
; A STOP condition or repeated START condition has been received while still addressed as Slave
; Enter not addressed mode and listen to address match, Enable TWI-interface and release TWI pins,
; Enable interrupt and clear the flag, Wait for new address match
	lds		R16, i2c_flags
	ori		R16, I2CF_IN_MSG			; set flag
	sts		i2c_flags, R16
	rjmp	ii_reset_intr
;
ii_skip60:
; DEBUG ++
	ldi		R16, 0x07
	sts		led_cntl, R16
;
; A General 00 address has been received..IGNORE
	rjmp	ii_reset_intr
;
ii_reset_intr:
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)
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
