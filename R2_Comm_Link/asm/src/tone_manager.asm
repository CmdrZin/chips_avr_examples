/*
 * Slave Manager Module
 *
 * org: 9/11/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: R2Link Demo Board w/ LED bank, 20MHz
 *
 * sig_generate:
 *   TONE_TIME
 *
 * Usage:
 * 	.include slave_manager.asm
 *
 */ 

// Tone Flags
.equ	TF_IN_MSG	= 0x01			; new message in Tone IN buffer

.DSEG
tone_buffer_in:			.BYTE	8	; Tone input buffer filled from goretz calcs.
tone_buffer_in_cnt:		.BYTE	1	; bytes in buffer.
;;tone_buffer_out:		.BYTE	8	; Tone output buffer
;;tone_buffer_out_cnt:	.BYTE	1	; bytes in buffer.

tone_flags:				.BYTE	1

.CSEG
/*
 * This function will generate a service based on the value in slave_state.
 * Call from main() and service every 10ms TIC.
 *
 * It will monitor Tone Symbol duration and rate for both Send and Receive.
 *
 * input reg:	none (slave_state)
 * output reg:	none
 *
 */
tone_service:
	lds		R16, slave_state

	ldi		XL, LOW(tone_buffer_in)
	ldi		XH, HIGH(tone_buffer_in)
;
	ld		R16, X+
	andi	R16, 0xF0	
	cpi		R16, MSG_TYPE_0
	brne	ts_skip00
; Type 0 msg

;
ts_skip00:
	ret

/*
 * Tone Communication Service Initialization
 *
 * input reg:	none
 * output reg:	none
 */
 tcomm_init:
	clr		R16
	sts		tgu_msg_status, R16
	ldi		R16, TONE_TIME
	sts		tgu_delay_cnt, R16
;
	ret

/*
 * Tone Communication Service
 * input reg:	none .. uses tgu_msg_buff
 * output reg:	none
 *
 * resources:
 *   tgu_msg_buff		Tone message buffer.
 *   tgu_msg_status		b3:0=byte count..b7=L/H nibble
 *
 * Read through the buffer and parse the bytes into 4bit values.
 * Output a new value every 100ms.
 *
 * If count = 0, EXIT.
 * Set up X, read X val, read status
 * if b7=0, get lower 4 bits of val
 * else get upper 4 bits and dec count..clear b7.
 *
 */
tcomm_service:
	sbis	GPIOR0, TCOMM_TIC		; test 10ms tic
	rjmp	tcs_exit				; EXIT..not set
	cbi		GPIOR0, TCOMM_TIC		; clear tic10ms flag set by interrup
;
	lds		R18, tgu_delay_cnt
	dec		R18
	sts		tgu_delay_cnt, R18		; update
;
	andi	R18, 0x7F				; mask out dead time bit.
	brne	tcs_exit
; check for dead time.
	lds		R18, tgu_delay_cnt
	sbrs	R18, 7					; Dead time if 1
	rjmp	tcs_skip10				; no..play tone
;
	ldi		R18, TONE_TIME
	sts		tgu_delay_cnt, R18		; update
	clt
	rjmp	tcs_exit
;
tcs_skip10:
	ldi		R18, TONE_TIME | 0x80	; next time is dead time
	sts		tgu_delay_cnt, R18		; update
// Run service
; Check count
	lds		R18, tgu_msg_status			; ...02,82,01,81,00,00,00
	tst		R18
	breq	tcs_exitHalt				; EXIT..no message data
;
	ldi		XH, high(tgu_msg_buff)		; set up X (R27:R26) = tgu_buffer
	ldi		XL, low(tgu_msg_buff)
	ldi		R17, 0x07					; get msg byt count
;
	dec		R18						; ADJUST
;
	and		R17, R18
	add		XL, R17						; X + R17
	clr		R17
	adc		XH, R17						; propagate cy
;
	mov		R17, R18
	andi	R17, 0x80					; test H/L bit
	breq	tcs_skip00
; high bits
	ld		R17, X
	andi	R17, 0xF0
	swap	R17
;
	inc		R18						; RESTORE
;
	dec		R18						; WALK backwards through msg buffer.
	andi	R18, 0x7F				; clear H/L bit
	sts		tgu_msg_status, R18		; update
	rjmp	tcs_skip01
;
tcs_skip00:
; low bits
	inc		R18						; RESTORE
;
	ori		R18, 0x80				; set H/L for next time.
	sts		tgu_msg_status, R18			; update
	ld		R17, X
	andi	R17, 0x0F
;
tcs_skip01:
	call	tgu_bits2tones
	call	tgu_init
	set
;
tcs_exit:
	ret

tcs_exitHalt:
	clt								; Stop auto tone gen.
;	call	tgu_zero_tone
;
; TEST .. Restart
/*
	clc
	lds		R17, tgu_msg_buff
	rol		R17
	sts		tgu_msg_buff, R17

	lds		R17, tgu_msg_buff+1
	rol		R17
	sts		tgu_msg_buff+1, R17

	lds		R17, tgu_msg_buff+2
	rol		R17
	sts		tgu_msg_buff+2, R17

	lds		R17, tgu_msg_buff+3
	rol		R17
	sts		tgu_msg_buff+3, R17
;
	brcc	tcs_tskip0
	lds		R17, tgu_msg_buff
	ori		R17, 0x01			; wrap cy bit
	sts		tgu_msg_buff, R17
;
tcs_tskip0:
	ldi		R17, 0x04
	sts		tgu_msg_status, R17
 */
;
	ret

/*
 * This function will generate a service based on the Mode.
 * Call when a Tone message is received.
 *
 * 1. Perform CKSUM test on Tone IN buffer.
 * 2. Copy to I2C OUT if not in ECHO mode.
 *
 * input reg:	none
 * output reg:	none
 *
 */
parse_tone_msg:
// Check CKSUM of Tone IN Buffer.
	ldi		XL, LOW(tone_buffer_in)
	ldi		XH, HIGH(tone_buffer_in)
	lds		R18, tone_buffer_in_cnt
	call	util_cksum					; X->CKSUM in buffer.
	ld		R18, X						; get buffer CKSUM
	cp		R17, R18
	breq	ptm_skip00
; CKSUM invalid
; Check for AUTO ACK/NACK
;** Load NACK symbol into Tone OUT and trigger to send. **
;
	rjmp	ptm_reset_tone_in			; reset Tone interface.
	ret
;
ptm_skip00:
	lds		R16, slave_mode
	ldi		R17, S_MODE_ECHO
	cp		R16, R17
	breq	ptm_reset_tone_in			; ignore Tone IN message
; NORMAL or other modes. Copy to I2C OUT buffer.
	lds		R16, i2c_buffer_in			; get Header
	andi	R16, 0x0F					; get num of bytes
	ldi		XL, LOW(tone_buffer_in)		; X=src
	ldi		XH, HIGH(tone_buffer_in)
	ldi		YL, LOW(i2c_buffer_out)		; Y=dst
	ldi		YH, HIGH(i2c_buffer_out)
;
ptm_loop0:
	ld		R17, X+
	st		Y+, R17
	dec		R16
	brne	ptm_loop0
;
	ld		R17, X						; get CKSUM
	st		Y, R17
; Check for AUTO ACK/NACK
;** Load ACK symbol into Tone OUT and trigger to send. **
;
	rjmp	ptm_reset_tone_in				; reset I2C IN Buffer to receive.
;
ptm_reset_tone_in:
;
	ret

