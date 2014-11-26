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
 * 	.include i2c_master.asm
 *
 */ 

// I2C Rate coefficients
.equ	I2C_CPU_CLOCK_20MHZ_TWBR	= 92
.equ	I2C_CPU_CLOCK_8MHZ_TWBR		= 32
.equ	I2C_CPU_CLOCK_1MHZ_TWBR		= 2


// I2C Communications State
.equ	I2C_COMM_IDLE	= 0		; No incoming or outgoing messages.
.equ	I2C_COMM_BUSY	= 1		; Sending or Receiving a message.
.equ	I2C_COMM_ERROR	= 2		; Last message sent or received had error(s).


// Interrupts state machine status values.
.equ	START = 0x08
.equ	REPT_START = 0x10		; Repeated START sent..treat as START
.equ	ARB_LOST = 0x38			; Arbitration Lost..FATAL ERROR

.equ	MT_SLA_ACK = 0x18		; SLA+W sent, ACK recv'd
.equ	MT_SLA_NACK = 0x20		; SLA+W sent, NACK recv'd

.equ	MT_DATA_ACK = 0x28		; Data sent, ACK recv'd
.equ	MT_DATA_NACK = 0x30		; Data sent, NACK recv'd

.equ	MR_SLA_ACK = 0x40		; SLA+R sent, ACK recv'd
.equ	MR_SLA_NACK = 0x48		; SLA+R sent, NACK recv'd

.equ	MR_DATA_ACK = 0x50		; Data recv'd, ACK recv'd
.equ	MR_DATA_NACK = 0x58		; Data recv'd, NACK recv'd on last byte?


.DSEG
i2c_slave_adrs:		.BYTE	1	; current Slave Address (non-adjusted)
i2c_comm_state:		.BYTE	1	; Current Comm State
								;	0:I2C_COMM_IDLE, 1:I2C_COMM_BUSY, 2:I2C_COMM_ERROR
i2c_comm_status:	.BYTE	1	; Current Comm Read Status. 0:Empty..N:Bytes read in.

i2c_error:			.BYTE	1	; error flags
								; Read:		b7:Ov,		b6: ArbLost,	b5: Res,	b4: CksumErr
								; Write:	b3:Res,		b2: ArbLost,	b1: Res,	b0: CksumErr


// Resume coding
.CSEG
/*
 * Initialize the TWI to support I2C interface as a Master
 * This is an interrupt driven system.
 *
 * input reg:	none
 * output reg:	none
 *
 * I2C rate = CPU/(16 + 2(TWBR)*(4^TWPS0:1)) = 1^6Hz / 20 = 100kHz
 * 20Mhz / 200 = 100kHz .. 16 + (2*92)*1) = 200
 * 8Mhz  / 80  = 100kHz .. 16 + (2*32)*1) = 80
 * 1Mhz  / 20  = 50kHz  .. 16 + (2*2)*1)  = 20
 *
 */
i2c_init_master:
	ldi		R16, I2C_CPU_CLOCK_20MHZ_TWBR	; Set I2C bit rate
	sts		TWBR, R16
;
	ldi		R16, 0							; Prescale TWPS1:0 = 00 .. 4^0 = 1
	sts		TWSR, R16
;
	clr		R16
	sts		TWDR, R16						; default data..release SDA line
;
	ldi		R16, (1<<TWEN)					; enable TWI pins
	sts		TWCR, R16
;
	ldi		R16, I2C_COMM_IDLE
	sts		i2c_comm_state, R16
;
	ret


/*
 * int i2c_checkState()					0: ok, 1: if BUSY, 2: ERROR
 *
 * output reg: 	R17 = i2c_comm_state
 *
 */
i2c_getState:
	lds		R17, i2c_comm_state
	ret

/*
 * int i2c_getErrorFlags()
 *
 * output reg: 	R17 = i2c_error
 */
i2c_getErrorFlags:
	lds		R17, i2c_error
	ret

/*
 * int i2c_getReadStatus()
 *
 * output reg: 	R17 = i2c_comm_status	0: None, 1: Read Message byte count
 *
 */
i2c_getReadStatus:
	lds		R17, i2c_comm_status
	ret

/*
 * Master has control of data transmissions.
 * int i2c_write( SlaveAdrs, *buffer, byteCount )
 * return: 0:ok, 1:if BUSY, 2:ERROR
 *
 * Buffer data: SLA_W, Data, ...
 * byteCount is total size of buffer.
 *
 * input reg:	R17 - Slave Address
 *				X - buffer
 *				R18 - byteCount
 *
 * output reg: 	R17 - results of call (not of message)
 *
 * resources:	R16, X
 *
 */
i2c_write:
; Check for I2C_COMM_IDLE(0)
	lds		R16, i2c_comm_state
	cpi		R16, I2C_COMM_IDLE
	breq	iw_skip00
	mov		R17, R16
	ret								; EXIT..not IDLE
;
iw_skip00:
// Save Slave Address (R17<<1)|(CTRL)) into TWDR Register.
	lsl		R17						; Format for SLA_W (R17<<1)|0)
	sts		i2c_slave_adrs, R17
; set X for use by intr service
	sts		i2c_XL, XL
	sts		i2c_XH, XH
; set output byte count
	sts		i2c_buff_out_cnt, R18
;
	ldi		R16, I2C_COMM_BUSY
	sts		i2c_comm_state, R16
// Issues START condition
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA)
	sts		TWCR, R16
;
	clr		R17
	ret


/*
 * Master has control of data transmissions.
 * int i2c_read( SlaveAdrs, *buffer, maxByteCount )
 * return: 0:ok, 1:if BUSY, 2:ERROR
 *
 * Buffer data: Data
 *
 * input reg:	R17 - Slave Address
 *				X - buffer
 *				R18 - byteCount
 *
 * output reg: 	R17 - results of call (not of message)
 *
 * resources:	R16 (R16)
 *
 */
i2c_read:
; Check for I2C_COMM_IDLE(0)
	lds		R16, i2c_comm_state
	cpi		R16, I2C_COMM_IDLE
	breq	ir_skip00
	mov		R17, R16
	ret								; EXIT..not IDLE
;
ir_skip00:
// Save Slave Address (R17<<1)|(CTRL)) into TWDR Register.
	sec
	rol		R17						; Format for SLA_R (R17<<1)|1)
	sts		i2c_slave_adrs, R17
; set X for use by intr service
	sts		i2c_XL, XL
	sts		i2c_XH, XH
; set input max byte count
	sts		i2c_buff_in_max, R18
;
	ldi		R16, I2C_COMM_BUSY
	sts		i2c_comm_state, R16
// Issues START condition
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA)
	sts		TWCR, R16
;
	clr		R17
	ret


/*
 * I2C interrupt service
 * 
 * resources: i2c_buff - data buffer
 *
 */
// Define a data buffer.
.equ	I2C_BUFF_IN_SIZE	= 18		; read Slave data into here.
.equ	I2C_BUFF_OUT_SIZE	= 18		; write Slave data from here.
;
.DSEG
i2c_XH:				.BYTE	1
i2c_XL:				.BYTE	1
;
i2c_buff_in_max:	.BYTE	1					; maximum input byte count.
i2c_buff_in_cnt:	.BYTE	1
i2c_buff_in:		.BYTE	I2C_BUFF_IN_SIZE	; dec to zero while receiving data
;
i2c_buff_out_cnt:	.BYTE	1					; dec to zero while sending data.
i2c_buff_out:		.BYTE	I2C_BUFF_OUT_SIZE

.CSEG
/*
 * I2C Interrupt Service
 * Supports both Write to Slave and Read from Slave
 * input reg:
 * output reg:
 *
 * Clear TWCR.TWINT by setting it to 1.
 */
ii_skip40:
	rjmp	ii_skip40e
ii_skip48:
	rjmp	ii_skip48e
ii_skip50:
	rjmp	ii_skip50e
ii_skip58:
	rjmp	ii_skip58e

i2c_intr:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	R16
	push	XL
	push	XH
;
	lds		R16, TWSR				; get status
	andi	R16, 0xF8				; mask out bits
; System level status words
	cpi		R16, START				; 0x08..START sent
	breq	ii_skip08
	cpi		R16, REPT_START			; 0x10..Repeated START sent..treat as START
	breq	ii_skip10
	cpi		R16, ARB_LOST			; 0x38..Arbitration Lost..FATAL ERROR
	breq	ii_skip38
; Transmit status words
	cpi		R16, MT_SLA_ACK			; 0x18..SLA+W sent, ACK recv'd
	breq	ii_skip18
	cpi		R16, MT_SLA_NACK		; 0x20..SLA+W sent, NACK recv'd
	breq	ii_skip20
	cpi		R16, MT_DATA_ACK		; 0x28..Data sent, ACK recv'd
	breq	ii_skip28
	cpi		R16, MT_DATA_NACK		; 0x30..Data sent, NACK recv'd
	breq	ii_skip30
; Receive status words
	cpi		R16, MR_SLA_ACK			; 0x40..SLA+R sent, ACK recv'd
	breq	ii_skip40
	cpi		R16, MR_SLA_NACK		; 0x48..SLA+R sent, NACK recv'd
	breq	ii_skip48
	cpi		R16, MR_DATA_ACK		; 0x50..Data recv'd, ACK sent
	breq	ii_skip50
	cpi		R16, MR_DATA_NACK		; 0x58..Data recv'd, NACK sent on last byte?
	breq	ii_skip58
; DEFAULT..error
	rjmp	ii_reset_intr			; Unexpected Status byte..FATAL Error
;
ii_skip08:
ii_skip10:
; START sent correctly. I2C lines are under Master control. Ok to send SLA_W or SLA_R.
; Get formatted SLA_x address to output. Same START is used for WRITE and READ.
; no need to check state
	lds		R16, i2c_slave_adrs
	sts		TWDR, R16				; load SLA_W or SLA_R adrs
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)
	sts		TWCR, R16				; trigger to send adrs
	rjmp	ii_exit
;
ii_skip20:
; Adrs sent, NACK recv'd..ERROR
	ldi		R16, 0xFF				; ARB ERROR CODE
	sts		i2c_error, R16
	rjmp	ii_reset_intr
;
ii_skip38:
; Arbitration Lost. Master no longer has control on I2C lines.
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA)
	sts		TWCR, R16				; restart
	rjmp	ii_exit
;
; *** WRITE OPERATIONS ***
ii_skip18:
; SLA+W sent, ACK recv'd
ii_skip28:
; Last Data sent, ACK recv'd
; Any data to send?
	lds		R16, i2c_buff_out_cnt		; track data bytes sent.
	tst		R16							
	breq	ii_skip29					; NO..STOP
; yes
	dec		R16
	sts		i2c_buff_out_cnt, R16		; update
; Sending data
	lds		XL, i2c_XL
	lds		XH, i2c_XH
	ld		R16, X+
	sts		TWDR, R16
; update X
	sts		i2c_XL, XL
	sts		i2c_XH, XH
;
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)
	sts		TWCR, R16				; trigger to send data
;
	rjmp	ii_exit
;
ii_skip29:
; send STOP and disable intr
	ldi		R16, (1<<TWEN)|(1<<TWINT)|(1<<TWSTO)
	sts		TWCR, R16
;
	ldi		R16, I2C_COMM_IDLE
	sts		i2c_comm_state, R16
; clean up
	ldi		R16, 0xFF
	sts		TWDR, R16
;
	rjmp	ii_exit
;
ii_skip30:
; Data sent, NACK recv'd..ERROR
	rjmp	ii_reset_intr					; ERROR..NACK adrs
;
; *** READ OPERATIONS ***
ii_skip40e:
; SLA+R sent, ACK recv'd
; reset count
	clr		R16
	sts		i2c_buff_in_cnt, R16	; clear input count
	sts		i2c_comm_status, R16	; clear status
; any space?
	lds		R16, i2c_buff_in_max
	tst		R16
	brne	ii_skip41				; yes
; no
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)	; NACK next send
	sts		TWCR, R16
	rjmp	ii_exit
;
ii_skip41:
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)	; ACK next send
	sts		TWCR, R16
	rjmp	ii_exit
;
ii_skip48e:
; SLA+R sent, NACK recv'd
	ldi		R16, 0xFF						; ARB ERROR CODE
	sts		i2c_error, R16
	rjmp	ii_reset_intr					; ERROR..NACK adrs
;
ii_skip50e:
; Data recv'd, ACK sent
// Read Data into buffer and check max
	lds		R17, TWDR
	lds		XL, i2c_XL
	lds		XH, i2c_XH
	st		X+, R17
; check for max
	lds		R16, i2c_buff_in_cnt
	inc		R16
	sts		i2c_buff_in_cnt, R16
	lds		R17, i2c_buff_in_max
	dec		R17								; adj to last byte
	cp		R17, R16
	brlt	ii_skip51						; Overflow..NACK
; update pointer
	sts		i2c_XL, XL
	sts		i2c_XH, XH
; Send ACK
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)
	sts		TWCR, R16
	rjmp	ii_exit
;
ii_skip51:
	ldi		R16, (1<<TWEN)|(1<<TWIE)|(1<<TWINT)	; NACK next send
	sts		TWCR, R16
	rjmp	ii_exit
;
ii_skip58e:
; Data recv'd, NACK sent on last byte
	lds		R17, TWDR
	lds		XL, i2c_XL
	lds		XH, i2c_XH
	st		X, R17
;
	ldi		R16, I2C_COMM_IDLE
	sts		i2c_comm_state, R16
; send STOP and disable intr
	ldi		R16, (1<<TWEN)|(1<<TWINT)|(1<<TWSTO)
	sts		TWCR, R16
; up[date status
	lds		R16, i2c_buff_in_cnt
	sts		i2c_comm_status, R16
	rjmp	ii_exit
;
; ERROR
ii_reset_intr:
	ldi		R16, (1<<TWEN)
	sts		TWCR, R16
; clean up
	ldi		R16, 0xFF
	sts		TWDR, R16
;
ii_exit:
	pop		XH
	pop		XL
	pop		R16
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti
