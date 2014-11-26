/*
 * Optocal IR Range Sensor Service
 *
 * org: 10/20/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Tank Bot Demo Board, 20MHz, ATmega164P
 *
 * This service supports four IR 2D04 range sensors.
 * The sensors use power control to help reduce power use which is 35ma per sensor.
 *
 * Dependentcies
 *   sys_timers.asm
 */

.equ	RNG_IR_LED_DELAY_COUNT	= 3		; ms..Sensor on time
.equ	RNG_IR_IDLE_DELAY_COUNT	= 30	; ms..scan delay


.equ	RNG_IR_WAIT_IDLE		= 0
.equ	RNG_IR_WAIT_LEFT_F		= 1
.equ	RNG_IR_WAIT_LEFT_R		= 2
.equ	RNG_IR_WAIT_RIGHT_F		= 3
.equ	RNG_IR_WAIT_RIGHT_R		= 4


.DSEG
range_ir_leftFront:		.BYTE	1		; 0cm = xxx..30cm = xxx
range_ir_leftRear:		.BYTE	1
range_ir_rightFront:	.BYTE	1
range_ir_rightRear:		.BYTE	1
range_ir_state:			.BYTE	1
range_ir_delay:			.BYTE	1		; 30ms nominal

.CSEG

/*
 * Initialize Optical Range Sensor
 *
 */
range_ir_service_init:
	call	range_ir_service_init_io
;
	ldi		R16, RNG_IR_IDLE_DELAY_COUNT
	sts		range_ir_delay, R16
	ldi		R16, RNG_IR_WAIT_IDLE
	sts		range_ir_state, R16
;
	ret

/*
 * Configure IO pins
 */
range_ir_service_init_io:

	ret

/*
 * range_service()
 *
 * input reg:	none
 *
 * output reg:	none
 *
 * resources:	range_ir_leftFront
 *				range_ir_leftRear
 *				range_ir_rightFront
 *				range_ir_rightRear
 *
 * Four ADC channels for IR distance.
 * Four I/O lines for sensor power conntrol.
 * Set up ADC to trigger after LED turn-on. 30ms cycle.
 * Cycle through sensors.
 *
 * Process
 * 0. Wait IDLE 300ms then Light Left Front LED for 30ms
 * 1. Wait 30ms then Sample Left Front Detector, Light Left Rear LED for 30ms
 * 2. Wait 30ms then Sample Left Rear Detector, Light Right Front LED for 30ms
 * 3. Wait 30ms then Sample Right Front Detector, Light Right Rear LED for 30ms
 * 4. Wait 30ms then Sample Right Rear Detector, Set IDLE delay for 300ms
 *
 */
range_ir_service:
	sbis	GPIOR0, RNG_10MS_TIC	; test 10ms tic
	ret								; EXIT..not set
;
	cbi		GPIOR0, RNG_10MS_TIC	; clear tic10ms flag set by interrup
// Run service
	lds		R16, range_ir_delay
	dec		R16
	sts		range_ir_delay, R16
	brne	rs_exit
; yes
	ldi		R16, RNG_IR_LED_DELAY_COUNT	; most states use LED time
	sts		range_ir_delay, R16
;
	lds		R16, range_ir_state		; get state
; switch(state)
	cpi		R16, RNG_IR_WAIT_IDLE
	brne	rs_skip00
; Leave IDLE
; Turn ON Left Sensor LED
;	cbi		PORTD, RNG_LED_LEFT
; Delay already set for 30ms
	ldi		R16, RNG_IR_WAIT_LEFT_F		; next state
	sts		range_ir_state, R16
	rjmp	rs_exit
;
rs_skip00:
	cpi		R16, RNG_IR_WAIT_LEFT_F
	brne	rs_skip10
; Sample Left Det
;;	ldi		R17, ADC_LEFT_CHAN
;;	call	adc_trigger				; returns R17.R18..left justified b9:2,b1:0
; Turn OFF Left Sensor LED
;	sbi		PORTD, RNG_LED_LEFT
; Turn ON Right Sensor LED
;	cbi		PORTD, RNG_LED_RIGHT
;
	ldi		R16, RNG_IR_WAIT_RIGHT_F
	sts		range_ir_state, R16
	rjmp	rs_exit
;
rs_skip10:
	cpi		R16, RNG_IR_WAIT_RIGHT_F
	brne	rs_skip20
; Sample Right Det
;;	ldi		R17, ADC_RIGHT_CHAN
;;	call	adc_trigger
; Set IDLE delay for 300ms
	ldi		R16, RNG_IR_IDLE_DELAY_COUNT
	sts		range_ir_delay, R16
;
rs_skip20:
; set to default
	ldi		R16, RNG_IR_WAIT_IDLE
	sts		range_ir_state, R16
rs_exit:
	ret
