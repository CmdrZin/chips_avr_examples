/*
 * Serial Library
 *
 * org: 11/03/2014
 * rev: 12/03/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: ATmega164P
 *
 * Basic utilities for serial supprt.
 *
 * Dependentcies:
 *   fifo_lib.asm
 *
 * NOTE: UART Registers are outside IN/OUT access range.
 *       Therefore, Data sheet examples don't work.
 */

.equ	SER_BUFF_SIZE	= 32

.equ	SER_STATUS_IDLE		= 0
.equ	SER_STATUS_SENDING	= 1

.DSEG
// Common struct used by FIFO utility.
ser_in_buff:		.BYTE	SER_BUFF_SIZE
ser_in_fifo_head:	.BYTE	1
ser_in_fifo_tail:	.BYTE	1

ser_out_buff:		.BYTE	SER_BUFF_SIZE
ser_out_fifo_head:	.BYTE	1
ser_out_fifo_tail:	.BYTE	1

ser_status:			.BYTE	1

.CSEG

/*
 * Initialize serial UART0 interface.
 */
serial_init:
; Set baud rate (9600 w/20MHz clock)
	ldi		r16, 0x01
	sts		UBRR0H, r16
	ldi		r16, 0x04
	sts		UBRR0L, r16
; Set doublle rate clock
	ldi		r16, (1<<U2X0)
	sts		UCSR0A, r16
; Enable RxIntr, Rx, Tx, 8 bit(2)
;;;	ldi		r16, (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0)
	ldi		r16, (1<<RXEN0)|(1<<TXEN0)
	sts		UCSR0B, r16
; Async, No Par, 1 Sbit, 8 bit(1:0)
	ldi		r16, (1<<UCSZ01)|(1<<UCSZ00)
	sts		UCSR0C, r16
;
	ldi		r16, SER_STATUS_IDLE
	sts		ser_status, r16
;
	clr		r16
	sts		ser_in_fifo_head, r16
	sts		ser_in_fifo_tail, r16
	sts		ser_out_fifo_head, r16
	sts		ser_out_fifo_tail, r16
;
	ret

/*
 * int serial_read( *char ) - Read one byte from input FIFO
 * r18 serial_read( &r17 ) - Read one byte from input FIFO
 *
 * input:	none
 * output:	r17		Data
 *			r18		Status..0:Valid, 1:Invalid
 */
serial_read_fifo:
	ldi		XL, LOW(ser_in_buff)
	ldi		XH, HIGH(ser_in_buff)
	ldi		r19, SER_BUFF_SIZE
	call	fifo_get
	ret

/*
 * void serial_write( char ) -  Write one byte into input FIFO
 * void serial_write( r17 ) -  Write one byte into input FIFO
 *
 * input:	r17
 * output:	none
 */
serial_write_fifo:
	ldi		XL, LOW(ser_out_buff)
	ldi		XH, HIGH(ser_out_buff)
	ldi		r19, SER_BUFF_SIZE
	call	fifo_put
 	ret

/*
 * void serial_send_byte()
 * Send one byte to Serial.
 *
 * input:	r17 - Data
 * output:	none
 */
serial_send_byte:
	lds		r16, UCSR0A
	sbrs	r16, UDRE0				; wait for xmit to empty
	rjmp	serial_send_byte
; Write to XMIT reg and trigger write.
	sts		UDR0, r17
	ret

/*
 * void serial_send_auto() -  Initiate serial message.
 * This will trigger the interrupt serial out until the output fifo is empty.
 *
 * input:	none
 * output:	none
 */
serial_send_auto:
; check IDLE
	lds		r16, ser_status
	cpi		r16, SER_STATUS_IDLE
	breq	ss_skip00
;;	ret								; EXIT..still busy
;
ss_skip00:
	ldi		r16, SER_STATUS_SENDING
	sts		ser_status, r16
; get first data byte
	ldi		XL, LOW(ser_out_buff)
	ldi		XH, HIGH(ser_out_buff)
	ldi		r19, SER_BUFF_SIZE
	call	fifo_get
; Write to XMIT reg and trigger write.
	ldi		r17, 0x55
	sts		UDR0, r17
	lds		r16, UCSR0B
	ori		r16, 1<<UDRIE0		; enable Data Empty intr to auto-load next byte.
	sts		UCSR0B, r16
;
	ret

/*
 * void serial_begin( rate, config ) -  Set rate and configuration
 *
 * Change configuration on UART0.
 *
 * input:	r17		Rate
 *			r18		bits, parity, stops
 *
 * output:	none
 *
 */
serial_begin:

	ret

/*
 * void serial_recv_byte()
 * Check for incoming byte from Serial.
 *
 * input:	none
 * output:	r17 - Data
 *			r18 - 0:valid..1:no data
 *
 */
serial_recv_byte:
	lds		r16, UCSR0A
	sbrs	r16, RXC0						; check recv
	rjmp	srb_exit
; Read data
	lds		r17, UDR0
;
srb_exit:
	ret

/*
 * void serial_recv_byte_blocking()
 * Wait for incoming byte from Serial.
 *
 * input:	none
 * output:	r17 - Data
 *
 */
serial_recv_byte_blocking:
	lds		r16, UCSR0A
	sbrs	r16, RXC0						; check recv
	rjmp	serial_send_byte
; Read data
	lds		r17, UDR0
	ret


/* **** Interrupt Services **** */

/* USART0, Rx Complete */
serial_rx_intr:
;	DISABLE
	reti
;
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	r16
	push	r19
	push	XL
	push	XH
;
; Read RCV register data -> r17
; Get and return received data from buffer
	lds		r16, UDR0
;
	ldi		XL, LOW(ser_in_buff)
	ldi		XH, HIGH(ser_in_buff)
	ldi		r19, SER_BUFF_SIZE
	call	fifo_put					; OV not checked
;
	push	XH
	push	XL
	push	r19
	push	r16
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti

/* USART0, Data register Empty */
serial_dr_intr:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	r16
	push	r17
	push	r18
	push	r19
	push	XL
	push	XH
; get next data byte
	ldi		XL, LOW(ser_out_buff)
	ldi		XH, HIGH(ser_out_buff)
	ldi		r19, SER_BUFF_SIZE
	call	fifo_get
; check
	tst		r18
	breq	sdi_skip00
; no data.. clear UDRIE0 bit
	lds		r16, UCSR0B
	andi	r16, ~(1<<UDRIE0)		; disable Data Empty intr.
	sts		UCSR0B, r16
;
	ldi		r16, SER_STATUS_IDLE
	sts		ser_status, r16
	rjmp	sdi_exit
; send next byte
sdi_skip00:
; Stuff data (r17) into XMIT reg and send
	sts		UDR0, r17
;
sdi_exit:
	push	XH
	push	XL
	push	r19
	push	r18
	push	r17
	push	r16
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti

