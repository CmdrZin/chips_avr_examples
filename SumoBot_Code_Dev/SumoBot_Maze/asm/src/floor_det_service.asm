/*
 * Sumo Maze Bot Floor Detector Code
 *
 * org: 01/21/2015
 * auth: Nels "Chip" Pearson
 *
 * Target: Sumo Maze Bot Board, 20MHz, ATmega164P, Sumo Bot base
 *
 * Uses ADC to measure detector output.
 *
 * Dependencies
 *	adc_triggered.asm
 *
 * Resources
 *	SRAM
 *
 * IO
 *
 */

.equ	FLOOR_DELAY_COUNT	= 10		; n * 1ms

.equ	FLOOR_LED_IO_RIGHT	= PORTD7
.equ	FLOOR_LED_IO_CENTER	= PORTD3
.equ	FLOOR_LED_IO_LEFT	= PORTD6

.equ	FLOOR_ADC_CHAN		= ADCH6
.equ	FLOOR_DET_LEVEL		= 40		; 0 - 255

; floor_state
.equ	FLOOR_STATE_IDLE	= 0
.equ	FLOOR_RIGHT_WAIT	= 10
.equ	FLOOR_RIGHT_READ	= 11
.equ	FLOOR_CENTER_WAIT	= 20
.equ	FLOOR_CENTER_READ	= 21
.equ	FLOOR_LEFT_WAIT		= 30
.equ	FLOOR_LEFT_READ		= 31

; floor_status
.equ	FLOOR_DET_RIGHT		= 0x01
.equ	FLOOR_DET_CENTER	= 0x02
.equ	FLOOR_DET_LEFT		= 0x04


.DSEG
floor_delay_cnt:	.BYTE	1			; service time delay * 10ms
floor_state:		.BYTE	1			; which LED/Detector to service
floor_det_status:	.BYTE	1			; Detector status..0=No det, 1=Detected


.CSEG
/*
 * Initialize Floor Service Parameters
 *
 * 
 */
floor_det_init:
	call	floor_det_init_io
;
	clr		r16
	sts		floor_det_status, r16
;
	ldi		r16, FLOOR_STATE_IDLE
	sts		floor_state, r16
;
	ldi		r16, FLOOR_DELAY_COUNT
	sts		floor_delay_cnt, r16
;
	ret

/*
 * Initialize Floor Detector IO
 * 
 */
floor_det_init_io:
; init IO output
	sbi		DDRD, FLOOR_LED_IO_RIGHT		; out
	sbi		PORTD, FLOOR_LED_IO_RIGHT		; OFF
	sbi		DDRD, FLOOR_LED_IO_CENTER
	sbi		PORTD, FLOOR_LED_IO_CENTER
	sbi		DDRD, FLOOR_LED_IO_LEFT
	sbi		PORTD, FLOOR_LED_IO_LEFT
;
	ret

/*
 * floor_det_service()
 *
 * Support for three IR sensors at front of Sumo Bot.
 *
 * Test 1ms flag
 * Cycle through three detectors.
 *	Turn on LED, wait 1ms, read ADC
 *
 */
floor_det_service:
	sbis	GPIOR0, FLOOR_1MS_TIC	; test 1ms tic
	ret								; EXIT..not set
;
	cbi		GPIOR0, FLOOR_1MS_TIC	; clear tic1ms flag set by interrup
// Run service
;
	lds		R16, floor_state
; switch(state)
	cpi		R16, FLOOR_STATE_IDLE
	brne	fds_skip10
; IDLE
	lds		r16, floor_delay_cnt
	dec		r16
	sts		floor_delay_cnt, r16
;
	breq	fds_skip00
	rjmp	fds_exit					; EXIT
;
fds_skip00:
	ldi		r16, FLOOR_DELAY_COUNT		; reset delay for next time
	sts		floor_delay_cnt, r16
; Turn ON RIGHT LED
	cbi		PORTD, FLOOR_LED_IO_RIGHT
; and setup for wait
	ldi		r16, FLOOR_RIGHT_WAIT
	sts		floor_state, r16
	rjmp	fds_exit					; EXIT
;
fds_skip10:
	cpi		R16, FLOOR_RIGHT_WAIT
	brne	fds_skip20
; RIGHT read sensor
	ldi		r17, FLOOR_ADC_CHAN
	call	adc_trigger					; BLOCKing call. Trigger and wait.
; Turn OFF RIGHT LED
	sbi		PORTD, FLOOR_LED_IO_RIGHT
; check and update status
	cpi		r17, FLOOR_DET_LEVEL
	brsh	fds_skip11
	lds		r16, floor_det_status
 	ori		r16, FLOOR_DET_RIGHT		; set flag
	rjmp	fds_skip12
;
fds_skip11:
	lds		r16, floor_det_status
 	andi	r16, ~FLOOR_DET_RIGHT		; clear flag
fds_skip12:
	sts		floor_det_status, r16		; update
;
; Turn ON CENTER LED
	cbi		PORTD, FLOOR_LED_IO_CENTER
; and setup for wait
	ldi		r16, FLOOR_CENTER_WAIT
	sts		floor_state, r16
	rjmp	fds_exit					; EXIT
;
fds_skip20:
	cpi		R16, FLOOR_CENTER_WAIT
	brne	fds_skip30
; CENTER read sensor
	ldi		r17, FLOOR_ADC_CHAN
	call	adc_trigger					; BLOCKing call. Trigger and wait.
;
; Turn OFF CENTER LED
	sbi		PORTD, FLOOR_LED_IO_CENTER
; check and update status
	cpi		r17, FLOOR_DET_LEVEL
	brsh	fds_skip21
	lds		r16, floor_det_status
 	ori		r16, FLOOR_DET_CENTER		; set flag
	rjmp	fds_skip22
;
fds_skip21:
	lds		r16, floor_det_status
 	andi	r16, ~FLOOR_DET_CENTER		; clear flag
fds_skip22:
	sts		floor_det_status, r16		; update
;
; Turn ON LEFT LED
	cbi		PORTD, FLOOR_LED_IO_LEFT
; and setup for wait
	ldi		r16, FLOOR_LEFT_WAIT
	sts		floor_state, r16
	rjmp	fds_exit					; EXIT
;
fds_skip30:
	cpi		R16, FLOOR_LEFT_WAIT
	brne	fds_skip31
; LEFT read sensor
	ldi		r17, FLOOR_ADC_CHAN
	call	adc_trigger					; BLOCKing call. Trigger and wait.
;
; Turn OFF LEFT LED
	sbi		PORTD, FLOOR_LED_IO_LEFT
; check and update status
	cpi		r17, FLOOR_DET_LEVEL
	brsh	fds_skip31
	lds		r16, floor_det_status
 	ori		r16, FLOOR_DET_LEFT			; set flag
	rjmp	fds_skip32
;
fds_skip31:
	lds		r16, floor_det_status
 	andi	r16, ~FLOOR_DET_LEFT		; clear flag
fds_skip32:
	sts		floor_det_status, r16		; update
;
; and setup for wait
	ldi		r16, FLOOR_STATE_IDLE
	sts		floor_state, r16
;
fds_exit:
	ret

