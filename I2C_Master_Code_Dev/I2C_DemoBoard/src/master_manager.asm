/*
 * Master Manager Module
 *
 * org: 9/11/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: I2C Demo Board w/ display and keypad I/O, 20MHz
 *
 * Usage:
 * 	.include master_manager.asm
 *
 */ 

// Message Type values
.equ	MSG_TYPE_0	= 0x00		; Send out as Echo Back message.
.equ	MSG_TYPE_1	= 0x10		; Send out as Tone message/Recv'd as Tone message.

// master_mode values
.equ	M_MODE_INVALID = 0
.equ	M_MODE_NORMAL = 1		; Send I2C message out.
.equ	M_MODE_ECHO = 2
.equ	M_MODE_RAW = 3
.equ	M_MODE_CAL = 4

// master_state values
.equ	MS_MODE_W_RD_ECHO	= 0		; Test. Writes EchoBack type then issues Read. Loops at 1ms rate.
.equ	MS_MODE_RD_DSP	= 1		; Read Slave and display bytes in sequence. Loops at 1ms rate.
								; Blank between each sequence. Alt dp between each data byte.
.equ	MS_MODE_LD_KP	= 2		; Load output buffer.
		; Every 2 keys are loaded as a byte. Show - - to indicate ready for next value.
		; Flast Number of Bytes in output buffer after each is added.
		; Press button to stop loading. Flash - - when done.
.equ	MS_MODE_DSP_IN	= 3		; Display input buffer. Show index then show byte. 3 sec delay between bytes.
.equ	MS_MODE_CLR_OUT	= 4		; Clear output buffer. Flash - - when done.
.equ	MS_MODE_CLR_IN	= 5		; Clear input buffer. Flash - - when done.

.equ	MS_MODE_SEND	= 16	; Send OUT buffer as Tone output message once per second.
.equ	MS_MODE_RECV	= 17	; Read Slave and Display message data. Each new byte overwrites.
								; Display - - if no data.

.DSEG
master_mode:	.BYTE	1		; current Master Mode state
master_state:	.BYTE	1		; current Master state
master_delay:	.BYTE	1		; extend delay

.CSEG
/*
 * This function will generate a service based on the value in master_mode.
 *
 * 1. Check for new KP data.
 *   If new key
 *      Check for A-D and set mode to that type. Goto service I2C input.
 *		Check for E(*) or F(#) and ignore for now.
 *      If 0-7, send I2C Type 0 message. Goto service I2C input.
 *	 Service I2C input.
 *	 Service I2C output.
 *
 * input reg:	none (master_mode)
 * output reg:	none
 *
 * KP Inputs
 * MODE:NORMAL
 */
master_service:
; Service KP Inputs
; Check for Mode change.
	lds		r16, kms_buff
	andi	r16, 0x80			; b7 test
	breq	ms_skip100			; no new kp data
; new KeyPad data
	lds		r17, kms_buff
	andi	r17, 0x0F			; b7 clear
	sts		kms_buff, R17		; update..consume data.
; switch(kp)
	cpi		r17, 0x0A
	brne	ms_skip01
	sts		dsp_buff+1, R17		; update display.
	ldi		r16, M_MODE_NORMAL
	sts		master_mode, r16
	rjmp	ms_skip100
;
ms_skip01:
	cpi		r17, 0x0B
	brne	ms_skip02
	sts		dsp_buff+1, R17		; update display.
	ldi		r16, M_MODE_ECHO
	sts		master_mode, r16
	rjmp	ms_skip100
;
ms_skip02:
	cpi		r17, 0x0C
	brne	ms_skip03
	sts		dsp_buff+1, R17		; update display.
	ldi		r16, M_MODE_RAW
	sts		master_mode, r16
	rjmp	ms_skip100
;
ms_skip03:
	cpi		r17, 0x0D
	brne	ms_skip04
	sts		dsp_buff+1, R17		; update display.
	ldi		r16, M_MODE_CAL
	sts		master_mode, r16
	rjmp	ms_skip100
;
ms_skip04:
	cpi		r17, 0x0E		; *
	cpi		r17, 0x0F		; #
;
; Process non-Mode keys 0-9
	lds		R16, master_mode		; get current mode
	cpi		R16, M_MODE_NORMAL
	brne	ms_skip90
; store KeyPad input to display
	sts		dsp_buff, R17			; update display.
	call	send_tone_msg
;
	rjmp	ms_skip100
;
ms_skip90:
	cpi		R16, M_MODE_ECHO
	cpi		R16, M_MODE_RAW
	cpi		R16, M_MODE_CAL
; Only need to check for mode change

ms_skip100:
; Service I2C Inputs

; Service I2C Outputs
	lds		R16, master_mode
; check 1ms TIC modes
	cpi		R16, M_MODE_ECHO
	brne	ms_skip110
; Writes EchoBack type then issues Read. Loops at 1ms rate.
 // Only service every 1ms
	sbis	GPIOR0, MS_1MS_TIC
	ret
;
	cbi		GPIOR0, MS_1MS_TIC
; TEST: Fill OUT buffer with test data and Type 0 message. Leave First byte for Address.
; NOTE: This buffer will be filled by other services later.
	ldi		XL, LOW(i2c_buff_out+1)
	ldi		XH, HIGH(i2c_buff_out+1)
	ldi		R16, MSG_TYPE_0 | 0x02	; 2 Byte message to return as echo.
	st		X+, R16
	ldi		R16, 0x24				; some data
	st		X+, R16
	lsr		R16
	st		X, R16
;
	ldi		R17, I2C_OP_WRITE
	call	i2c_service
	ret								; EXIT
;
ms_skip110:
;
; check 10ms TIC modes
ms_skip160:
	cpi		R16, M_MODE_RAW
	brne	ms_skip170
 // Only service every 1000ms
	sbis	GPIOR0, MS_10MS_TIC
	ret
;
	cbi		GPIOR0, MS_10MS_TIC
	lds		R16, master_delay
	dec		R16
	sts		i2c_delay, R16
	breq	ms_skip161					; run service
	ret
;
ms_skip161:
; Send Read RAW I2C output message once per second.
	ldi		R16, 100
	sts		i2c_delay, R16
;
	ldi		R16, 4
	sts		i2c_buff_in_cnt, R16	; set max count to recv
	ldi		R17, I2C_OP_READ
	call	i2c_service
	ret								; EXIT
;
ms_skip170:
;
ms_exit:
	ret


/*
 * This function will load a Type 0 msg into the I2C OUT buffer.
 * All messages are 4 bytes.
 *
 * input reg:	Msg Index
 * output reg:	none
 *
 * MODE: NORMAL
 */
send_tone_msg:
; setup Table pointer
	andi	R17, 0x07					; mask off valid data. 0-7
	ldi		ZH, high(tone_msg_table<<1)	; Initialize Z pointer
	ldi		ZL, low(tone_msg_table<<1)
	lsl		R17							; index * 4
	lsl		R17
	add		ZL, R17						; add offset
	clr		R16
	adc		ZH, R16						; propigate carry bit
; setup I2C pointer.
	ldi		XL, LOW(i2c_buff_out+1)		; R26
	ldi		XH, HIGH(i2c_buff_out+1)	; R27
;
	lpm		R17, Z+						; Load pattern
	st		X+, R17						;
	lpm		R17, Z+						; Load pattern
	st		X+, R17						;
	lpm		R17, Z+						; Load pattern
	st		X+, R17						;
	lpm		R17, Z+						; Load pattern
	st		X+, R17						;
;
	ldi		R17, I2C_OP_WRITE
	call	i2c_service
;
	ret

// Four byte messages.
tone_msg_table:
//     Header, D0,   D1,   D2
.db		0x03, 0x12, 0x34, 0x56
.db		0x03, 0xFE, 0xDC, 0xBA
.db		0x03, 0x0F, 0x1E, 0x2C
.db		0x03, 0x67, 0x89, 0xAB
.db		0x03, 0xAB, 0xCD, 0xEF
.db		0x03, 0x70, 0x1B, 0x36
.db		0x03, 0x92, 0x1A, 0x63
.db		0x03, 0xE3, 0x85, 0x27

