/*
 * Slave Manager Module
 *
 * org: 9/11/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: R2Link Demo Board w/ LED bank, 20MHz
 *
 * Usage:
 * 	.include slave_manager.asm
 *
 */ 

// Message Type values
.equ	MSG_TYPE_0	= 0x00		; Normal Data Parcket. N=1-15 bytes.
/*	SLAVE_ECHO:
 *		Copy valid Type 0 I2C messages to I2C OUT. Ignores Tone messages.
 *	Any Mode other than SLAVE_ECHO:
 *		Copy the message bytes into its Tone OUT buffer and output.
 */
.equ	MSG_TYPE_F	= 0xF0		; Set Parameters. Data pairs. N*2 bytes.
/*	Any Mode:
 *		Slave will parse data pairs and set parameters.
 */
.equ	PARAM_LOOP		= 0x00		; Set Output Loops 0:No Loop, 1->FE, FF:Continuous
.equ	PARAM_AUTOA		= 0x01		; Set ACK/NACK response..0:No, 1:Yes
.equ	PARAM_SYMTDLY	= 0x02		; Timeout Delay. N=Symbol delays to wait for next Symbol.
.equ	PARAM_SYMRATE	= 0x03		; Symbol Rate. Rate to send Symbols. N=n * 10ms.
.equ	PARAM_SYMDUR`	= 0x04		; Symbol Duration. Time to hold Tone output. N=n * 10 ms.
.equ	PARAM_MODE		= 0x06		; Set Mode (1-4)

// Slave Modes
.equ	S_MODE_INVALID		= 0x00
.equ	S_MODE_NORMAL		= 0x01
.equ	S_MODE_ECHO			= 0x02
.equ	S_MODE_RAW			= 0x03
.equ	S_MODE_CAL			= 0x04


// slave_state values
.equ	SS_ECHO		= 2		; Do not load OUT buffer from Tones.

// Slave Control Flags
.equ	SCF_NO_TONE_OUT	= 0x01	; If set=1, inhibit Tone generation
.equ	SCF_NO_TONE_IN	= 0x02	; If set=1, do not process ADC sample with Goeritz


.DSEG
slave_mode:		.BYTE	1		; current Slave Mode
slave_flags:	.BYTE	1		; subsystem control flags
slave_state:	.BYTE	1		; current Slave Mode state
slave_delay:	.BYTE	1		; extend delay

.CSEG

/*
 * This function will generate a service based on the Mode and slave_state.
 * Call when an I2C message is received from Master.
 *
 * 1. Perform CKSUM test on I2C IN buffer.
 * 2. Parse based on Mode.
 *
 * input reg:	none
 * output reg:	none
 *
 */
parse_i2c_msg:
	rjmp	tst_00

// Check CKSUM of I2C IN Buffer.
	ldi		XL, LOW(i2c_buffer_in)
	ldi		XH, HIGH(i2c_buffer_in)
	lds		R18, i2c_buffer_in_cnt
	call	util_cksum					; X->CKSUM in buffer.
	ld		R18, X						; get buffer CKSUM
	cp		R17, R18
	breq	pim_skip00
; CKSUM invalid
	rjmp	pim_reset_in				; reset I2C IN Buffer to receive.
;
pim_skip00:
// CKSUM valid. Check for Type F.
	lds		R16, i2c_buffer_in			; get Header
	andi	R16, 0xF0					; mask
	cpi		R16, MSG_TYPE_F
	brne	pim_skip10
; Type F
	ldi		R16, S_MODE_NORMAL
	sts		slave_mode, R16				; Default Mode = NORMAL
// For now, ANY Type F msg wil set the Mode to NORMAL.
// TODO: Parse Data Pairs of Parameters from msg.
;
	rjmp	pim_reset_in				; reset I2C IN Buffer to receive.
;
pim_skip10:
// Check for Type 0.
	cpi		R16, MSG_TYPE_0
	brne	pim_reset_in				; EXIT..Others Types not supported.
;
	lds		R16, slave_mode
	cpi		R16, S_MODE_NORMAL
	brne	pim_skip20
; In NORMAL mode, Type 0 msgs are sent to the Tone OUT buffer.
	lds		R16, i2c_buffer_in			; get Header
	andi	R16, 0xF0					; mask
	cpi		R16, MSG_TYPE_0
	brne	pim_skip10
;
tst_00:
; Type 0: Data .. Copy to Tone Msg Out
	lds		R16, i2c_buffer_in			; get Header
	andi	R16, 0x0F					; get num of bytes
	ldi		XL, LOW(i2c_buffer_in)		; X=src
	ldi		XH, HIGH(i2c_buffer_in)
	ldi		YL, LOW(tgu_msg_buff)		; Y=dst
	ldi		YH, HIGH(tgu_msg_buff)
;
pim_loop00:
	ld		R17, X+
	st		Y+, R17
	dec		R16
	brne	pim_loop00
;
	ld		R17, X						; get CKSUM
	st		Y, R17
; Trigger TONE Out
	lds		R16, i2c_buffer_in			; get Header
	andi	R16, 0x0F					; get num of bytes
	sts		tgu_msg_status, R16			; Set byte count.
;
	rjmp	pim_reset_in				; reset I2C IN Buffer to receive.
;
pim_skip20:
	cpi		R16, S_MODE_ECHO
	brne	pim_skip100
; In ECHO Mode, Type 0 messages go back into I2C OUT buffer.
	lds		R16, i2c_buffer_in			; get Header
	andi	R16, 0x0F					; get num of bytes
	sts		i2c_buffer_out_cnt, R16		; store byte count.
	ldi		XL, LOW(i2c_buffer_in)		; X=src
	ldi		XH, HIGH(i2c_buffer_in)
	ldi		YL, LOW(i2c_buffer_out)		; Y=dst
	ldi		YH, HIGH(i2c_buffer_out)
;
pim_loop10:
	ld		R17, X+
	st		Y+, R17
	dec		R16
	brne	pim_loop10
;
	ld		R17, X						; get CKSUM
	st		Y, R17
	rjmp	pim_reset_in				; reset I2C IN Buffer to receive.
;
pim_skip100:
; Test for other Modes when supported.
;
pim_reset_in:
	lds		R16, i2c_flags
	cbr		R16, I2CF_IN_MSG
	sts		i2c_flags, R16
	call	i2c_slave_init				; reset I2C interface.
;
	ret

/*
 * This function will generate a service based on the Mode and slave_state.
 * Call from main() loop to service periodic operations.
 *
 * Check slave_mode
 *   S_MODE_NORMAL:		Copy Type 0 I2C IN msg to Tone OUT buffer
 *						Ignore other types except Type F.
 *   S_MODE_ECHO:		Copy Type 1 I2C IN msg to I2C OUT buffer
 *						Ignore other types except Type F.
 *   S_MODE_RAW:		Ignore I2C IN msg types except Type F.
 *						Copy new ADC output to I2C OUT buffer
 *   S_MODE_CAL:		Ignore I2C IN msg types except Type F.
 *
 * input reg:	none (slave_state)
 * output reg:	none
 *
 */
slave_service:
// Check for ADC sample. Always Service ADC.
	call	adc_get_data		; read ADC buffer..ADC intr driven
	tst		R18					; check for new data.
	brne	ss_skip00
; Process?
	lds		R16, slave_flags
	andi	R16, SCF_NO_TONE_IN
	brne	ss_skip00
;
	call	goertzel_1_tone		; process data.
	lds		R16, gortz_det
	sts		led_bank, R16
;
ss_skip00:
// Check for I2C In Message. Always process Type F to set parameters.
	lds		R16, i2c_flags
	andi	R16, I2CF_IN_MSG		; ckeck flag
	breq	ss_skip10				; not set
; New. Parse I2C message.
	call	parse_i2c_msg			; Parse is based on Mode.
;
	lds		R16, i2c_flags
	cbr		R16, I2CF_IN_MSG		; clear flag
	sts		i2c_flags, R16
;
ss_skip10:
// Process MODE specific tasks
	lds		R16, slave_mode
	cpi		R16, S_MODE_NORMAL
	brne	ss_skip20
; NORMAL Mode
// Check for Tone In Message if NOT in ECHO mode
	lds		R16, tone_flags
	andi	R16, TF_IN_MSG
	breq	ss_skip20
;
	call	parse_tone_msg
;
ss_skip20:
	cpi		R16, S_MODE_ECHO
	brne	ss_skip30
; ECHO Mode

;
ss_skip30:
	cpi		R16, S_MODE_RAW
	brne	ss_skip40
; RAW Mode

;
ss_skip40:
	cpi		R16, S_MODE_CAL
	brne	ss_skip50
; CAL Mode

;
ss_skip50:
// Service any 1ms TIC operations
	sbis	GPIOR0, MS_1MS_TIC
	ret
;
	cbi		GPIOR0, MS_1MS_TIC
	ret								; EXIT

