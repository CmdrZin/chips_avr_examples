/*
 * Sonar Range Sensor Service
 *
 * org: 11/12/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Tank Bot Demo Board, 20MHz, ATmega164P
 *
 * This service supports three ultrasonic HR04 range sensors.
 *
 * 56.4us/cm .. Max range: 300cm .. Max wait: 17ms (use 20ms)
 *
 * Dependentcies
 *   sys_timers.asm
 */

.equ	RNG_S_IDLE_DELAY_COUNT	= 5		; 50ms..Sensor idle scan
.equ	RNG_S_MAX_WAIT_COUNT	= 5		; 50ms..Maximum ping wait delay 38ms

.equ	RNG_S_WAIT_IDLE			= 0
.equ	RNG_S_WAIT_LEFT			= 1
.equ	RNG_S_WAIT_CENTER		= 2
.equ	RNG_S_WAIT_RIGHT		= 3

; SONAR uses PORTC.2:7
.equ	SONAR_RIGHT_TRIG	= PORTC2
.equ	SONAR_RIGHT_ECHO	= PORTC3
.equ	SONAR_CENTER_TRIG	= PORTC4
.equ	SONAR_CENTER_ECHO	= PORTC5
.equ	SONAR_LEFT_TRIG		= PORTC6
.equ	SONAR_LEFT_ECHO		= PORTC7

; Pin masks
.equ	SONAR_RIGHT_MASK	= (1<<PORTC3)
.equ	SONAR_CENTER_MASK	= (1<<PORTC5)
.equ	SONAR_LEFT_MASK		= (1<<PORTC7)


.DSEG
range_s_left:		.BYTE	1		; 0cm = xxx..150cm = xxx
range_s_center:		.BYTE	1
range_s_right:		.BYTE	1
range_s_state:		.BYTE	1
range_s_delay:		.BYTE	1		; 30ms nominal
range_s_transit:	.BYTE	1
range_s_pin:		.BYTE	1		; pin in use

.CSEG

/*
 * Initialize Sonar Range Sensor
 *
 * PCICR = (1<<PCIE2) .. other bit are 0.
 *
 */
range_s_service_init:
	call	range_s_service_init_io
;
	ldi		R16, RNG_S_IDLE_DELAY_COUNT
	sts		range_s_delay, R16
	ldi		R16, RNG_S_WAIT_IDLE
	sts		range_s_state, R16
; Set up INTR on change
	ldi		r16, (1<<PCIE2)
	sts		PCICR, r16
;
	ret

/*
 * Configure IO pins
 */
range_s_service_init_io:
	sbi		DDRC, SONAR_RIGHT_TRIG		// output
	cbi		DDRC, SONAR_RIGHT_ECHO		// input
;
	sbi		DDRC, SONAR_CENTER_TRIG	// output
	cbi		DDRC, SONAR_CENTER_ECHO	// input
;
	sbi		DDRC, SONAR_LEFT_TRIG		// output
	cbi		DDRC, SONAR_LEFT_ECHO		// input
;
	ret

/* Ping trigger delay 10us */
range_s_trigger_wait:
	push	r16
;
	ldi		r16, 40
; 0.05us @ 20MHz
rstw_loop:
	nop
	nop
	dec		r16
	brne	rstw_loop
;
	pop		r16
	ret

/*
 * range_s_service()
 *
 * input reg:	none
 *
 * output reg:	none
 *
 * resources:	range_s_left
 *				range_s_center
 *				range_s_right
 *
 * Three I/O lines for triggers.
 * Three I/O lines for interrupt response.
 * Cycle through sensors.
 *
 * Process
 * 0. Wait IDLE 100ms..Trigger Left..delay MAX
 * 1. Sample Left Detector..Trigger Center..delay MAX
 * 2. Sample Center Detector..Trigger Right..delay MAX
 * 3. Sample Right Detector..delay IDLE
 *
 */
range_sonar_service:
	sbis	GPIOR0, RNG_S_10MS_TIC		; test 10ms tic
	ret									; EXIT..not set
;
	cbi		GPIOR0, RNG_S_10MS_TIC		; clear tic10ms flag set by interrupt
; check delay
	lds		r16, range_s_delay
	dec		r16
	sts		range_s_delay, r16
	breq	rss_skip00
	ret									; EXIT..not time
;
rss_skip00:
; State servcie sets next delay value.
// Run service
; Get MODE
	lds		r16, range_s_state
; switch(state)
	cpi		r16, RNG_S_WAIT_IDLE
	brne	rss_skip10
; Trigger Right
	sbi		PORTC, SONAR_LEFT_TRIG
	call	range_s_trigger_wait		; wait 10us
	cbi		PORTC, SONAR_LEFT_TRIG
;
	ldi		r16, (1<<SONAR_LEFT_ECHO)	; enable pin interrupt
	sts		PCMSK2, r16
	ldi		r16, SONAR_LEFT_MASK
	sts		range_s_pin, r16
;
	ldi		r16, RNG_S_MAX_WAIT_COUNT
	sts		range_s_delay, r16
;
	ldi		r16, RNG_S_WAIT_LEFT
	rjmp	rss_exit
;
rss_skip10:
	cpi		r16, RNG_S_WAIT_LEFT
	brne	rss_skip20
; Record LEFT..Counter is limited to 255 max distance.
	lds		r16, range_s_transit
	sts		range_s_left, r16
; Trigger Center
	sbi		PORTC, SONAR_CENTER_TRIG
	call	range_s_trigger_wait		; wait 10us
	cbi		PORTC, SONAR_CENTER_TRIG
;
	ldi		r16, (1<<SONAR_CENTER_ECHO)	; enable pin interrupt
	sts		PCMSK2, r16
	ldi		r16, SONAR_CENTER_MASK
	sts		range_s_pin, r16
;
	ldi		r16, RNG_S_MAX_WAIT_COUNT
	sts		range_s_delay, r16
;
	ldi		r16, RNG_S_WAIT_CENTER
	rjmp	rss_exit
;
rss_skip20:
	cpi		r16, RNG_S_WAIT_CENTER
	brne	rss_skip30
; Record CENTER..Counter is limited to 255 max distance.
	lds		r16, range_s_transit
	sts		range_s_center, r16
; Trigger Right
	sbi		PORTC, SONAR_RIGHT_TRIG
	call	range_s_trigger_wait		; wait 10us
	cbi		PORTC, SONAR_RIGHT_TRIG
;
	ldi		r16, (1<<SONAR_RIGHT_ECHO)	; enable pin interrupt
	sts		PCMSK2, r16
	ldi		r16, SONAR_RIGHT_MASK
	sts		range_s_pin, r16
;
	ldi		r16, RNG_S_MAX_WAIT_COUNT
	sts		range_s_delay, r16
;
	ldi		r16, RNG_S_WAIT_RIGHT
	rjmp	rss_exit
;
rss_skip30:
	cpi		r16, RNG_S_WAIT_RIGHT
	brne	rss_skip40
; Record RIGHT..Counter is limited to 255 max distance.
	lds		r16, range_s_transit
	sts		range_s_right, r16
;
	ldi		r16, RNG_S_IDLE_DELAY_COUNT
	sts		range_s_delay, r16
;
rss_skip40:
	ldi		r16, RNG_S_WAIT_IDLE		; default..ERROR
rss_exit:
	sts		range_s_state, r16			; update state
	ret


/*
 * PCINT2 Change (PORTC pin change)
 * The monitored pin has changed.
 *
 * Check pin (range_s_pin)
 *   HIGH:	zero timer
 *   LOW:	copy timer and disable pin interrupt
 *
 */
range_s_intr:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	r16
	push	r17
;
	in		r16, PINC					; Read I/O pin
	lds		r17, range_s_pin
	and		r16, r17
	breq	rsi_skip10
; HIGH
	clr		r16
	sts		st_tmr2_count, r16			; clear return time count
	rjmp	rsi_exit
;
rsi_skip10:
; LOW
	lds		r16, st_tmr2_count
	sts		range_s_transit, r16
	clr		r16
	sts		PCMSK2, r16					; disable pin interrupt
rsi_exit:
	pop		r17
	pop		r16
;
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti
