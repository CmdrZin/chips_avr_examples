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
 * Uses PORT A
 *
 * Dependentcies
 *   sys_timers.asm
 *   adc_util_triggered.asm
 *
 */

.equ	RNG_IR_LED_DELAY_COUNT	= 3		; 30ms..Sensor on time
.equ	RNG_IR_IDLE_DELAY_COUNT	= 8		; 80ms..scan delay


.equ	RNG_IR_WAIT_IDLE		= 0
.equ	RNG_IR_WAIT_LEFT_F		= 1
.equ	RNG_IR_WAIT_LEFT_R		= 2
.equ	RNG_IR_WAIT_RIGHT_F		= 3
.equ	RNG_IR_WAIT_RIGHT_R		= 4


; IR Range uses PORTA.0:7
.equ	IR_LEFT_FRONT_SIG	= 0
.equ	IR_LEFT_REAR_SIG	= 1
.equ	IR_RIGHT_REAR_SIG	= 2
.equ	IR_RIGHT_FRONT_SIG	= 3

.equ	IR_LEFT_FRONT_CTRL	= PORTA4
.equ	IR_LEFT_REAR_CTRL	= PORTA5
.equ	IR_RIGHT_REAR_CTRL	= PORTA6
.equ	IR_RIGHT_FRONT_CTRL	= PORTA7


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
; Disable all
	sbi		PORTA, IR_LEFT_FRONT_CTRL
	sbi		PORTA, IR_LEFT_REAR_CTRL
	sbi		PORTA, IR_RIGHT_REAR_CTRL
	sbi		PORTA, IR_RIGHT_FRONT_CTRL
; Set as output
	sbi		DDRA, IR_LEFT_FRONT_CTRL
	sbi		DDRA, IR_LEFT_REAR_CTRL
	sbi		DDRA, IR_RIGHT_REAR_CTRL
	sbi		DDRA, IR_RIGHT_FRONT_CTRL
; Setup ADC for channels 0-3 by adc_init_hdwr()
	ret

/*
 * range_ir_service()
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
	sbis	GPIOR0, RNG_10MS_TIC		; test 10ms tic
	ret									; EXIT..not set
;
	cbi		GPIOR0, RNG_10MS_TIC		; clear tic10ms flag set by interrupt
;
	lds		R16, range_ir_delay
	dec		R16
	sts		range_ir_delay, R16
	breq	ris_skip00
	rjmp	rs_exit
ris_skip00:
; Run Service
	lds		R16, range_ir_state			; get state
; switch(state)
	cpi		R16, RNG_IR_WAIT_IDLE
	brne	ris_skip10
; Leave IDLE
; Turn ON Left Front IR Sensor
	cbi		PORTA, IR_LEFT_FRONT_CTRL
; Set next delay
	ldi		R16, RNG_IR_LED_DELAY_COUNT
	sts		range_ir_delay, R16
; next state
	ldi		R16, RNG_IR_WAIT_LEFT_F
	sts		range_ir_state, R16
	rjmp	rs_exit
;
ris_skip10:
	cpi		R16, RNG_IR_WAIT_LEFT_F
	brne	rs_skip20
; Sample Left Front Range
	ldi		R17, IR_LEFT_FRONT_SIG
	call	adc_trigger					; returns R17.R18..left justified b9:2,b1:0
	sts		range_ir_leftFront, r17		; Only use upper 8bits.
; Turn OFF Left Front IR Sensor
	sbi		PORTA, IR_LEFT_FRONT_CTRL
; Turn ON Left Rear IR Sensor
	cbi		PORTA, IR_LEFT_REAR_CTRL
; Set next delay
	ldi		R16, RNG_IR_LED_DELAY_COUNT
	sts		range_ir_delay, R16
; next state
	ldi		R16, RNG_IR_WAIT_LEFT_R
	sts		range_ir_state, R16
	rjmp	rs_exit
;
rs_skip20:
	cpi		R16, RNG_IR_WAIT_LEFT_R
	brne	rs_skip30
; Sample Left Front Range
	ldi		R17, IR_LEFT_REAR_SIG
	call	adc_trigger					; returns R17.R18..left justified b9:2,b1:0
	sts		range_ir_leftRear, r17		; Only use upper 8bits.
; Turn OFF Left Read IR Sensor
	sbi		PORTA, IR_LEFT_REAR_CTRL
; Turn ON RIGHT Front IR Sensor
	cbi		PORTA, IR_RIGHT_FRONT_CTRL
; Set next delay
	ldi		R16, RNG_IR_LED_DELAY_COUNT
	sts		range_ir_delay, R16
; next state
	ldi		R16, RNG_IR_WAIT_RIGHT_F
	sts		range_ir_state, R16
	rjmp	rs_exit
;
rs_skip30:
	cpi		R16, RNG_IR_WAIT_RIGHT_F
	brne	rs_skip40
; Sample Right Front Range
	ldi		R17, IR_RIGHT_FRONT_SIG
	call	adc_trigger					; returns R17.R18..left justified b9:2,b1:0
	sts		range_ir_rightFront, r17	; Only use upper 8bits.
; Turn OFF Right Front IR Sensor
	sbi		PORTA, IR_RIGHT_FRONT_CTRL
; Turn ON RIGHT Rear IR Sensor
	cbi		PORTA, IR_RIGHT_REAR_CTRL
; Set next delay
	ldi		R16, RNG_IR_LED_DELAY_COUNT
	sts		range_ir_delay, R16
; next state
	ldi		R16, RNG_IR_WAIT_RIGHT_R
	sts		range_ir_state, R16
	rjmp	rs_exit
;
rs_skip40:
	cpi		R16, RNG_IR_WAIT_RIGHT_R
	brne	rs_skip50
; Sample Right Rear Range
	ldi		R17, IR_RIGHT_REAR_SIG
	call	adc_trigger					; returns R17.R18..left justified b9:2,b1:0
	sts		range_ir_rightRear, r17		; Only use upper 8bits.
; Turn OFF Right Rear IR Sensor
	sbi		PORTA, IR_RIGHT_REAR_CTRL
; Set next delay
	ldi		R16, RNG_IR_IDLE_DELAY_COUNT
	sts		range_ir_delay, R16
; next state
	ldi		R16, RNG_IR_WAIT_IDLE
	sts		range_ir_state, R16
	rjmp	rs_exit
;
rs_skip50:
; set to default
	ldi		R16, RNG_IR_WAIT_IDLE
	sts		range_ir_state, R16
rs_exit:
	ret
