/*
 * Optocal Range Sensor Service
 *
 * org: 10/20/2014
 * auth: Nels "Chip" Pearson
 * rev: 01/26/2015
 *
 * Target: Sumo Bot Demo Board, 8MHz w/65ms reset delay,
 *
 * Resources
 * SRAM
 *
 * IO
 *
 */

.equ	RNG_LED_DELAY_COUNT		= 1		; 1ms
.equ	RNG_IDLE_DELAY_COUNT	= 100	; 100ms

.equ	RNG_LED_LEFT_R			= PORTC7
.equ	RNG_LED_LEFT_F			= PORTC6
.equ	RNG_LED_CENTER_L		= PORTC5
.equ	RNG_LED_CENTER_R		= PORTC4
.equ	RNG_LED_RIGHT_F			= PORTC3
.equ	RNG_LED_RIGHT_R			= PORTC2

.equ	RNG_DET_LEFT_R			= 0
.equ	RNG_DET_LEFT_F			= 1
.equ	RNG_DET_CENTER_L		= 2
.equ	RNG_DET_CENTER_R		= 3
.equ	RNG_DET_RIGHT_F			= 4
.equ	RNG_DET_RIGHT_R			= 5

.equ	RNG_WAIT_IDLE			= 0
.equ	RNG_WAIT_LEFT_R			= 1
.equ	RNG_WAIT_LEFT_F			= 2
.equ	RNG_WAIT_CENTER_L		= 3
.equ	RNG_WAIT_CENTER_R		= 4
.equ	RNG_WAIT_RIGHT_F		= 5
.equ	RNG_WAIT_RIGHT_R		= 6


.DSEG
range_left_R:	.BYTE	1		; 0x00=xxcm..0xF0=xxcm
range_left_F:	.BYTE	1		; 0x00=xxcm..0xF0=xxcm
range_center_L:	.BYTE	1
range_center_R:	.BYTE	1
range_right_R:	.BYTE	1
range_right_F:	.BYTE	1
range_temp:		.BYTE	1

range_state:	.BYTE	1
range_delay:	.BYTE	1		; 30ms nominal

.CSEG


/*
 * Initialize Optical Range Sensor
 *
 */
range_service_init:
	call	range_service_init_io
;
	ldi		R16, RNG_IDLE_DELAY_COUNT
	sts		range_delay, R16
	ldi		R16, RNG_WAIT_IDLE
	sts		range_state, R16
;
	ret

/*
 * Configure IO pins
 */
range_service_init_io:
	sbi		PORTC, RNG_LED_LEFT_R
	sbi		PORTC, RNG_LED_LEFT_F
	sbi		PORTC, RNG_LED_CENTER_L
	sbi		PORTC, RNG_LED_CENTER_R
	sbi		PORTC, RNG_LED_RIGHT_F
	sbi		PORTC, RNG_LED_RIGHT_R
;
	ret

/*
 * range_service()
 *
 * Display text on display.
 *
 * input reg:	none
 *
 * output reg:	none
 *
 * resources:	range_left_R, range_left_F
 *				range_center_L, range_center_R
 *				range_right_R, range_right_F
 *
 * Six ADC channels for IR distance.
 * Set up ADC to trigger after LED turn-on. 1ms cycle.
 *
 * Process
 * 0. Wait IDLE 100ms then Sample and Light Left_R LED for 1ms
 * 1. Wait 1ms then Sample Left_R Detector, Calc Diff, Sample and Light Left_F LED for 1ms
 * 1. Wait 1ms then Sample Left_F Detector, Calc Diff, Sample and Light Center_L LED for 1ms
 * 1. Wait 1ms then Sample Center_L Detector, Calc Diff, Sample and Light Center_R LED for 1ms
 * 1. Wait 1ms then Sample Center_R Detector, Calc Diff, Sample and Light Right_F LED for 1ms
 * 1. Wait 1ms then Sample Right_F Detector, Calc Diff, Sample and Light Right_R LED for 1ms
 * 1. Wait 1ms then Sample Right_R Detector. Calc Diff, and Set IDLE delay for 100ms
 *
 * NOTE: This could be done as a table driven system.
 *
 */
range_service:
	sbis	GPIOR0, RNG_1MS_TIC
	ret								; EXIT..not set
;
	cbi		GPIOR0, RNG_1MS_TIC		; clear tic1ms flag set by interrup
// Run service
	lds		R16, range_delay
	dec		R16
	sts		range_delay, R16
	breq	rs_skip00
	rjmp	rs_exit					; EXIT
;
rs_skip00:
	ldi		R16, RNG_LED_DELAY_COUNT	; most states use LED time
	sts		range_delay, R16
;
	lds		R16, range_state		; get state
; switch(state)
	cpi		R16, RNG_WAIT_IDLE
	brne	rs_skip10
; Leave IDLE
;
; Sample Left_R Det for baseline
	ldi		r17, RNG_DET_LEFT_R
	call	adc_trigger				; returns R17.R18..left justified b9:2,b1:0
	sts		range_temp, r17			; save baseline
; Turn ON Left_R Sensor LED
	sbi		PORTC, RNG_LED_LEFT_R
	ldi		R16, RNG_WAIT_LEFT_R		; next state
	sts		range_state, R16
	rjmp	rs_exit
;
rs_skip10:
	cpi		R16, RNG_WAIT_LEFT_R
	brne	rs_skip20
; Sample Left_R Det
	ldi		r17, RNG_DET_LEFT_R
	call	adc_trigger				; returns R17.R18..left justified b9:2,b1:0
; Turn OFF Left_R Sensor LED
	cbi		PORTC, RNG_LED_LEFT_R
; calc diff
	lds		r16, range_temp
	sub		r17, r16
; average
	lds		r16, range_left_R
	add		r17, r16
	ror		r17
	sts		range_left_R, r17
;
; Sample Left_F Det for baseline
	ldi		R17, RNG_DET_LEFT_F
	call	adc_trigger
	sts		range_temp, r17			; save baseline
; Turn ON Left_F Sensor LED
	sbi		PORTC, RNG_LED_LEFT_F
	ldi		r16, RNG_WAIT_LEFT_F
	sts		range_state, r16
	rjmp	rs_exit
;
rs_skip20:
	cpi		R16, RNG_WAIT_LEFT_F
	brne	rs_skip30
; Sample Left_F Det
	ldi		R17, RNG_DET_LEFT_F
	call	adc_trigger
; Turn OFF Left_R Sensor LED
	cbi		PORTC, RNG_LED_LEFT_F
; calc diff
	lds		r16, range_temp
	sub		r17, r16
; average
	lds		r16, range_left_F
	add		r17, r16
	ror		r17
	sts		range_left_F, r17
;
; Sample Center_L Det for baseline
	ldi		r17, RNG_DET_CENTER_L
	call	adc_trigger
	sts		range_temp, r17			; save baseline
; Turn ON Center_L Sensor LED
	sbi		PORTC, RNG_LED_CENTER_L
	ldi		r16, RNG_WAIT_CENTER_L
	sts		range_state, r16
	rjmp	rs_exit
;
rs_skip30:
	cpi		R16, RNG_WAIT_CENTER_L
	brne	rs_skip40
; Sample Center_L Det
	ldi		r17, RNG_DET_CENTER_L
	call	adc_trigger
; Turn OFF Center_L Sensor LED
	cbi		PORTC, RNG_LED_CENTER_L
; calc diff
	lds		r16, range_temp
	sub		r17, r16
; average
	lds		r16, range_center_L
	add		r17, r16
	ror		r17
	sts		range_center_L, r17
;
; Sample Center_R Det for baseline
	ldi		r17, RNG_DET_CENTER_R
	call	adc_trigger
	sts		range_temp, r17			; save baseline
; Turn ON Center_R Sensor LED
	sbi		PORTC, RNG_LED_CENTER_R
	ldi		r16, RNG_WAIT_CENTER_R
	sts		range_state, r16
	rjmp	rs_exit
;
rs_skip40:
	cpi		R16, RNG_WAIT_CENTER_R
	brne	rs_skip50
; Sample Center_R Det
	ldi		r17, RNG_DET_CENTER_R
	call	adc_trigger
; Turn OFF Center_R Sensor LED
	cbi		PORTC, RNG_LED_CENTER_R
; calc diff
	lds		r16, range_temp
	sub		r17, r16
; average
	lds		r16, range_center_R
	add		r17, r16
	ror		r17
	sts		range_center_R, r17
;
; Sample Right_F Det for baseline
	ldi		r17, RNG_DET_RIGHT_F
	call	adc_trigger
	sts		range_temp, r17			; save baseline
; Turn ON Right_F Sensor LED
	sbi		PORTC, RNG_LED_RIGHT_F
	ldi		r16, RNG_WAIT_RIGHT_F
	sts		range_state, r16
	rjmp	rs_exit
;
rs_skip50:
	cpi		R16, RNG_WAIT_RIGHT_F
	brne	rs_skip60
; Sample Right_F Det
	ldi		r17, RNG_DET_RIGHT_F
	call	adc_trigger
; Turn OFF Right_F Sensor LED
	cbi		PORTC, RNG_LED_RIGHT_F
; calc diff
	lds		r16, range_temp
	sub		r17, r16
; average
	lds		r16, range_right_F
	add		r17, r16
	ror		r17
	sts		range_right_F, r17
;
; Sample Right_R Det for baseline
	ldi		r17, RNG_DET_RIGHT_R
	call	adc_trigger
	sts		range_temp, r17			; save baseline
; Turn ON Right_R Sensor LED
	sbi		PORTC, RNG_LED_RIGHT_R
	ldi		r16, RNG_WAIT_RIGHT_R
	sts		range_state, r16
	rjmp	rs_exit
;
rs_skip60:
	cpi		R16, RNG_WAIT_RIGHT_R
	brne	rs_skip70
; Sample Right_R Det
	ldi		r17, RNG_DET_RIGHT_R
	call	adc_trigger
; Turn OFF Right_R Sensor LED
	cbi		PORTC, RNG_LED_RIGHT_R
; calc diff
	lds		r16, range_temp
	sub		r17, r16
; average
	lds		r16, range_right_R
	add		r17, r16
	ror		r17
	sts		range_right_R, r17
;
; Set IDLE delay for 300ms
	ldi		R16, RNG_IDLE_DELAY_COUNT
	sts		range_delay, R16
;
rs_skip70:
; set to default
	ldi		R16, RNG_WAIT_IDLE
	sts		range_state, R16
;
rs_exit:
	ret
