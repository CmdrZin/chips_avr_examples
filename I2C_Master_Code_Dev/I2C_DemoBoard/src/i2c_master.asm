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

.equ	SLAVE_ADRS	= (0x57)	; default


.equ	START = 0x08
.equ	REPT_START = 0x10		; Repeated START sent..treat as START
.equ	ARB_LOST = 0x30			; Arbitration Lost..FATAL ERROR

.equ	MT_SLA_ACK = 0x18		; SLA+W sent, ACK recv'd
.equ	MT_SLA_NACK = 0x20		; SLA+W sent, NACK recv'd

.equ	MT_DATA_ACK = 0x28		; Data sent, ACK recv'd
.equ	MT_DATA_NACK = 0x30		; Data sent, NACK recv'd

.equ	MR_SLA_ACK = 0x40		; SLA+R sent, ACK recv'd
.equ	MR_SLA_NACK = 0x48		; SLA+R sent, NACK recv'd

.equ	MR_DATA_ACK = 0x50		; Data recv'd, ACK recv'd
.equ	MR_DATA_NACK = 0x58		; Data recv'd, NACK recv'd on last byte?

.equ	I2C_OP_WRITE = 0		; Write Adrs->Data, ...->CKSUM
.equ	I2C_OP_READ  = 1		; Read Adrs<-Data, ... <-CKSUM



.DSEG
i2c_delay:			.BYTE	1
i2c_status:			.BYTE	1				; current I2C status
											; b0: 0=Idle, 1=Busy. Executing a Write or Read operation.
											; b1: Input buffer. 0=Empty, 1=Has data .. read header for size.
											; b2: Output buffer. 0=Empty, 1=Has data .. read header for size.

											; b7: 0=Ok, 1=No ACKs .. Failing due to no Slave ACKs.
;
i2c_state:			.BYTE	1				; current I2C state
											; 0=Idle
											; 1=START sent..wait for (0x08) START sent
											; 2=SLA_W sent..wait for (0x18) ACK, or (0x20) NACK, (0x38) Arb Lost.FAIL
											; 3=Data sent.. wait for (0x28) ACK, or (0x30) NACK, (0x38) Arb Lost.FAIL

											; b7 - Operation: 0=SLA_W..1=SLA_R

;
;
i2c_error:			.BYTE	1				; error flag..b0:OV
											; 0xFF..ARB ERROR CODE
;
i2c_test:			.BYTE	1

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
 */
i2c_init_master:
	ldi		R16, 92
	sts		TWBR, R16
;
	ldi		R16, 0			; Prescale TWPS1:0 = 00 .. 4^0 = 1
	sts		TWSR, R16
;
	lds		R16, TWCR
	ori		R16, (1<<TWIE)|(1<<TWEN)	; enable intr on TWI state change.
	sts		TWCR, R16
;
	clr		R16
	sts		i2c_state, R16
	sts		i2c_error, R16
	sts		i2c_buff_in_cnt, R16
	sts		i2c_buff_out_cnt, R16
;
	ret

/*
 * Service I2C Master
 *
 * input reg:	R17 Mode	I2C_OP_WRITE (0)	I2C_OP_READ (1)
 *				i2c_buffer_out:	Data to send
 *				i2c_buffer_out_cnt:	Number of Data Bytes (max 15)
 *
 *				i2c_buffer_in:	Data recv'd
 *				i2c_buffer_in_cnt:	Expected number of Data Bytes (max 15)
 *
 * output reg: 	none
 * resources:
 *
 *
 */
i2c_service:
	tst		R17							; I2C_OP_WRITE (0)
	brne	i2c_skip00
; Write Adrs->Data, ...->CKSUM
	ldi		R17, (SLAVE_ADRS<<1)|0x00	; send to Slave Adrs 0xXX+WRITE(b0=0)
	call	i2c_write
	ret
;
i2c_skip00:								; I2C_OP_READ (1)
; Read Adrs<-Data, ... <-CKSUM
	ldi		R17, (SLAVE_ADRS<<1)|0x01	; send to Slave Adrs 0xXX+READ(b0=1)
	lds		R18, i2c_buff_in_cnt		; max data. _cnt is decremented during read intr process
	call	i2c_read
;
i2c_skip03:
	ret

/*
 * Master has control of data transmissions.
 * Buffer data: SLA_W, (Type+Nbytes), Data, ..., CKSUM
 *
 * input reg:	R17 - Slave Address
 * output reg: 	none
 * resources:	R16, X
 *
 */
i2c_write:
	// Load Slave Address (R17<<1)|(CTRL)) into TWDR Register.
	ldi		XL, LOW(i2c_buff_out)	; R26
	ldi		XH, HIGH(i2c_buff_out)	; R27
; update X for use by intr service
	sts		i2c_XL, XL
	sts		i2c_XH, XH
	st		X+, R17
; Get byte count
	ld		R18, X					; Leave X -> Header byte.
	andi	R18, 0x0F
	inc		R18						; adj for added CKSUM byte.
	sts		i2c_buff_out_cnt, R18	; Decremented by intr for byte count.
	dec		R18						; adj back to data count only.
	call	util_cksum				; Calculate CKSUM = (CKSUM ^ Data) ROL 1..Includes Header
									; X returns pointing to CKSUM location in buffer.
	st		X, R17					; store CKSUM at end of buffer.
// Issues START condition
	ldi		R16, (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE)
	sts		TWCR, R16
;
	ret

/*
 * Master has control of data transmissions.
 * input reg:	R18 - Slave Address
 * output reg: 	R17 - Data
 *				R18 - error state  0:ok
 * resources:	R16 (R16)
 *
 */
i2c_read:
	// Issues START condition
	ldi		R16, (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE)
	sts		TWCR, R16
	// This indicates that the START condition has been transmitted.
i2c_rd_wait1:
	lds		R16, TWCR
	sbrs	R16, TWINT
	rjmp	i2c_rd_wait1
	// Check value of TWI Status Register. Mask prescaler bits.
	//  If status different from START go to ERROR.
	lds		R16, TWSR
	andi 	R16, 0xF8
	cpi		R16, START
	brne	i2c_rd_ERROR

	// Load Slave Address (R18<<1)|(CTRL)) into TWDR Register.
	// Clear TWINT bit in TWCR to start transmission of address.
	sts		TWDR, R18
	ldi		R16, (1<<TWINT) | (1<<TWEN)|(1<<TWIE)
	sts		TWCR, R16
	// Wait for TWINT Flag set. This indicates that the SLA+R has been
	// transmitted, and ACK/NACK has been received.
i2c_rd_wait2:
	lds		R16, TWCR
	sbrs	R16, TWINT
	rjmp	i2c_rd_wait2
	// Check value of TWI Status Register.
	// Mask prescaler bits. If status different from MT_SLA_ACK go to ERROR
	lds		R16, TWSR
	andi	R16, 0xF8
	cpi		R16, MR_SLA_ACK
	brne	i2c_rd_ERROR
	// Get Data (R17) from TWDR Register.
	// Clear TWINT bit in TWCR to start transmission of data.
	ldi		R16, (1<<TWINT) | (1<<TWEN)|(1<<TWIE)
	sts		TWCR, R16
	// Wait for TWINT Flag set. This indicates that the DATA has been
	// transmitted, and ACK/NACK has been received.
i2c_rd_wait3:
	lds		R16, TWCR
	sbrs	R16, TWINT
	rjmp	i2c_rd_wait3
	// Check value of TWI Status Register.
	// Mask prescaler bits. If status different from MR_DATA_ACK go to ERROR
	lds		R16, TWSR
	andi	R16, 0xF8
	cpi		R16, MR_DATA_ACK			; 0x50
	cpi		R16, MR_DATA_NACK			; 0x58..ok also
;
;	mov		dsp_buff, R16				; 0x58 NACK??
;
; HACK	brne	i2c_rd_ERROR
	lds		R17, TWDR
	// Transmit STOP condition
	ldi		R16, (1<<TWINT)|(1<<TWEN)|(1<<TWSTO)
	sts		TWCR, R16
	// EXIT..ok
	clr		R18
; clean up?
	ldi		R16, 0xFF
	sts		TWDR, R16
;
	ret
	// EXIT..error
i2c_rd_ERROR:
	// Transmit STOP condition. Should always clean up.
	ldi		R16, (1<<TWINT)|(1<<TWEN)|(1<<TWSTO)
	sts		TWCR, R16
	// set error state
	ldi		R18, $FF
	ret

/*
 * Master has control of data transmissions.
 * input reg:	none
 * output reg:	R17 - Data
 *				R18 - Data valid. 0:valid..1:no data
 *				R19 - Error code. 0:pass..1:read errors
 */
i2c_get_data:
	call	fifo_get
	ret

/*
 * Clear error flag
 * input reg:	none
 * output reg:	none
 */
 i2c_clear:
 	clr		R16
	sts		i2c_error, R16
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
ii_skip35:
	rjmp	ii_skip35e
ii_skip40:
	rjmp	ii_skip40e
ii_skip45:
	rjmp	ii_skip45e
ii_skip50:
	rjmp	ii_skip50e

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
	breq	ii_skip00
	cpi		R16, REPT_START			; 0x10..Repeated START sent..treat as START
	breq	ii_skip00
	cpi		R16, ARB_LOST			; 0x38..Arbitration Lost..FATAL ERROR
	breq	ii_skip10
; Transmit status words
	cpi		R16, MT_SLA_ACK			; 0x18..SLA+W sent, ACK recv'd
	breq	ii_skip15
	cpi		R16, MT_SLA_NACK		; 0x20..SLA+W sent, NACK recv'd
	breq	ii_skip20
	cpi		R16, MT_DATA_ACK		; 0x28..Data sent, ACK recv'd
	breq	ii_skip25
	cpi		R16, MT_DATA_NACK		; 0x30..Data sent, NACK recv'd
	breq	ii_skip30
; Receive status words
	cpi		R16, MR_SLA_ACK			; 0x40..SLA+R sent, ACK recv'd
	breq	ii_skip35
	cpi		R16, MR_SLA_NACK		; 0x48..SLA+R sent, NACK recv'd
	breq	ii_skip40
	cpi		R16, MR_DATA_ACK		; 0x50..Data recv'd, ACK recv'd
	breq	ii_skip45
	cpi		R16, MR_DATA_NACK		; 0x58..Data recv'd, NACK recv'd on last byte?
	breq	ii_skip50
; DEFAULT..error
	rjmp	ii_reset_intr			; Unexpected Status byte..FATAL Error
;
ii_skip00:
; START sent correctly. I2C lines are under Master control. Ok to send SLA_W or SLA_R.
	lds		XL, i2c_XL
	lds		XH, i2c_XH
; no need to check state
	ld		R16, X+
	sts		TWDR, R16				; load SLA_W or SLA_R adrs
	ldi		R16, (1<<TWINT) | (1<<TWEN)|(1<<TWIE)
	sts		TWCR, R16				; trigger to send adrs
; update X
	sts		i2c_XL, XL
	sts		i2c_XH, XH
;
	rjmp	ii_exit
;
ii_skip10:
; Arbitration Lost. Master no longer has control on I2C lines.
	ldi		R16, 0xFF				; ARB ERROR CODE
	sts		i2c_error, R16
;
	rjmp	ii_reset_intr
;
; Write operations
ii_skip15:
; Adrs sent, ACK recv'd..Sending data
	lds		XL, i2c_XL
	lds		XH, i2c_XH
ii_bkup00:
	ld		R16, X+
	sts		TWDR, R16				; load data
	ldi		R16, (1<<TWINT) | (1<<TWEN)|(1<<TWIE)
	sts		TWCR, R16				; trigger to send data
; update X
	sts		i2c_XL, XL
	sts		i2c_XH, XH
;
	rjmp	ii_exit
;
ii_skip20:
; Adrs sent, NACK recv'd..Resend data
ii_skip30:
; Data sent, NACK recv'd..Resend data
;; TEST
	ldi		R16, 9
	sts		dsp_buff, R16
;; TEST
	lds		XL, i2c_XL
	lds		XH, i2c_XH
	ld		R16, -X					; adjust X
	rjmp	ii_bkup00				; resend
;
ii_skip25:
; Data sent, ACK recv'd..Sending data
	lds		R16, i2c_buff_out_cnt			; track data bytes sent.
	dec		R16
	sts		i2c_buff_out_cnt, R16
	breq	ii_skip_26						; done?
; no 
	rjmp	ii_skip15						; send next byte.
;
ii_reset_intr:
; Error clean up is STOP and FLUSH.
ii_skip_26:
; yes..send STOP
	ldi		R16, (1<<TWINT)|(1<<TWEN)|(1<<TWSTO)
	sts		TWCR, R16
; clean up
	ldi		R16, 0xFF
	sts		TWDR, R16
;
	rjmp	ii_exit
;
; Read Operaions
ii_skip35e:
ii_skip40e:
ii_skip45e:
ii_skip50e:
	rjmp	ii_reset_intr					; Temp reset for now.
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

.DSEG
i2c_head:		.BYTE	1
i2c_tail:		.BYTE	1

.CSEG
.equ	FIFO_SIZE = I2C_BUFF_IN_SIZE

/* Utilites */
/*
 * Get one byte from FIFO
 * input reg:	none
 * output reg:	R17
 *				R18 - Data valid. 0:valid..1:no data
 * resources:	i2c_buff - FIFO data buffer
 *				FIFO_SIZE - numbytes in FIFO
 *				i2c_head - in index to FIFO
 *				i2c_tail - out index to FIFO
 */
fifo_get:
		// setup FIFO address pointer - X reg
		ldi		XL, LOW(i2c_buff_in)		; R26
		ldi		XH, HIGH(i2c_buff_in)		; R27
		lds		R17, i2c_head
		lds		R18, i2c_tail
		cp		R17, R18				; test for empty
		brne	fg001					; skip if data	
		ldi		R18, $1					; no data
		ret								; EXIT
fg001:
		// 16 bit add..X + i2c_tail
		clc
		adc		XL, R18
		clr		R16
		adc		XH, R16
		ld		R17, X					; get data
		// Update i2c_tail
		inc		R18
		sts		i2c_tail, R18
		// check for wrap around
		ldi		R16, FIFO_SIZE
		cp		R18, R16
		brne	fg002
		// at end
		clr		R18
		sts		i2c_tail, R18			; reset to begining
fg002:
		clr		R18
		ret

/*
 * Put one byte into FIFO
 * NOTE: Overflow tested. If FULL, don't store. Set i2c_error OV=1.
 * input reg:	R17
 * output reg:	none
 * resources:	i2c_buff - FIFO data buffer
 *				FIFO_SIZE - numbytes in FIFO
 *				i2c_error - error flags
 *				i2c_head - in index to FIFO
 *				i2c_tail - out index to FIFO
 *
 * NOTE: Called from INTR service routine. SAVE regs.
 */
fifo_put:
		push	R16
		push	XL
		push	XH
;
		// setup FIFO address pointer - X reg
		ldi		XL, LOW(i2c_buff_in)
		ldi		XH, HIGH(i2c_buff_in)
		// 16 bit add..X + i2c_head
		lds		R16, i2c_head
		adc		XL, R16
		clr		R16
		adc		XH, R16
		st		X, R17				; put data
		// Update i2c_tail
		lds		R17, i2c_head		; save in case of ov.
		inc		R17
		sts		i2c_head, R17
		// check for wrap around
		ldi		R16, FIFO_SIZE
		cp		R17, R16
		brne	fp001
		// at end
		clr		R17
		sts		i2c_head, R17		; reset
fp001:
		lds		R16, i2c_tail
		cp		R17, R16			; test for overflow
		brne	fp002				; skip if ov
		sts		i2c_head, R17		; restore i2c_head
		ldi		R16, $1
		sts		i2c_error, R16
fp002:
		pop		XH
		pop		XL
		pop		R16
;
		ret
