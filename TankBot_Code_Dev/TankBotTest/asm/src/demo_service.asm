/*
 * Tank Bot Demo Code
 *
 * org: 11/13/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Tank Bot Demo Board, 20MHz, ATmega164P. Tank Bot
 *
 * Dependentcies
 *   pwm_dc_motor_lib
 *   sys_timers
 *   range_ir_service
 *   range_sonar_service
 */

.equ	DEMO_DELAY_COUNT	= 1		; 10msec

.equ	DEMO_MODE_SQUARE	= 1
.equ	DEMO_MODE_UPBACK	= 2
.equ	DEMO_MODE_AVOID		= 3		; avoid obsticals

// SQUARE
.equ	DEMO_SQUARE_FWD			= 0		; go forward
.equ	DEMO_SQUARE_TURN90		= 2		; turn right

// AVOID
.equ	DEMO_AVOID_FWD		= 0		; go forward
.equ	DEMO_AVOID_CONTINUE	= 1		; continue forward
.equ	DEMO_AVOID_BACKUP	= 2		; go reverse
.equ	DEMO_AVOID_TURN180	= 3		; spin in place


.DSEG
demo_delay:		.BYTE	1
demo_mode:		.BYTE	1		; 0=Square, 1=Up-Back
demo_state:		.BYTE	1		; mode state
demo_s_time:	.BYTE	1		; state time

.CSEG

tank_demo_init:
	ldi		R16, DEMO_DELAY_COUNT
	sts		demo_delay, R16
;
	ldi		R16, DEMO_MODE_SQUARE
;;	ldi		R16, DEMO_MODE_AVOID
	sts		demo_mode, R16
;
	ret


/*
 * Some simple state machines to demonstrate to system.
 */
tank_demo:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	ret									; EXIT..not set
;
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
// Run service every 10ms
; Get MODE
	lds		R16, demo_mode
; switch(mode)
	cpi		R16, DEMO_MODE_SQUARE
	brne	sd_skip10
	lds		R16, demo_state
// DEMO - SQUARE
; switch(state)
	cpi		R16, DEMO_SQUARE_FWD
	brne	sd_skip010
; Done with last state?
	lds		R16, demo_s_time
	dec		R16
	sts		demo_s_time, R16			; update
	breq	sd_skip001
	rjmp	sd_exit						; no
sd_skip001:
; Go forward
	ldi		R17, DIR_FWD
	call	pwm_set_right_dir
	call	pwm_set_left_dir
; at medium speed
	ldi		R17, PWM_R_MED
	call	pwm_set_right
	ldi		R17, PWM_L_MED
	call	pwm_set_left
;
	ldi		R16, 200
	sts		demo_s_time, R16			; run for 2 sec
;
	ldi		R16, DEMO_SQUARE_TURN90
	sts		demo_state, R16				; next state
	rjmp	sd_exit
;
sd_skip010:
	cpi		R16, DEMO_SQUARE_TURN90
	brne	sd_skip020
; Done with last state?
	lds		R16, demo_s_time
	dec		R16
	sts		demo_s_time, R16			; update
	breq	sd_skip011
	rjmp	sd_exit						; no
; yes
sd_skip011:
; Turn Left
	call	pwm_stop_left
;
	ldi		R16, 105					; Tune this to turn 90 degrees
	sts		demo_s_time, R16			; run for 2 sec
;
	ldi		R16, DEMO_SQUARE_FWD
	sts		demo_state, R16					; next state
	rjmp	sd_exit
;
sd_skip020:
	clr		R16
	sts		demo_state, R16				; reset to default
	rjmp	sd_exit
;
sd_skip10:
	cpi		R16, DEMO_MODE_UPBACK
	brne	sd_skip20
	ret									; not supported
;
sd_skip20:
	cpi		R16, DEMO_MODE_AVOID
	breq	sd_skip21
	rjmp	sd_skip30
;
sd_skip21:
// DEMO - AVOID
	lds		R16, demo_state
; switch(state)
	cpi		R16, DEMO_AVOID_FWD
	brne	sd_skip210
; Done with last state?
	lds		R16, demo_s_time
	dec		R16
	sts		demo_s_time, R16			; update
	breq	sd_skip201
	rjmp	sd_exit
;
sd_skip201:
; Go forward
	ldi		R17, DIR_FWD
	call	pwm_set_right_dir
	call	pwm_set_left_dir
; at medium speed
	ldi		R17, PWM_R_MED
	call	pwm_set_right
	ldi		R17, PWM_L_MED
	call	pwm_set_left
;
	ldi		R16, DEMO_AVOID_CONTINUE
	sts		demo_state, R16				; next state
	rjmp	sd_exit
;
sd_skip210:
	cpi		R16, DEMO_AVOID_CONTINUE
	brne	sd_skip220
; Check detectors..ALL must be zero
	call	obstical_check
	tst		r17							; 0: no obsticals
	brne	sd_skip211
	rjmp	sd_exit						; ok to continue
; Obstical detected
sd_skip211:
; Backup at SLOW speed
	ldi		R17, PWM_L_SLOW
	call	pwm_set_left
	ldi		R17, PWM_R_SLOW
	call	pwm_set_right
;
	ldi		R17, DIR_REV
	call	pwm_set_right_dir
	call	pwm_set_left_dir
;
	ldi		R16, 60
	sts		demo_s_time, R16		; run for 600ms
;
	ldi		R16, DEMO_AVOID_BACKUP	; start back up and turn around
	sts		demo_state, R16			; next state
	rjmp	sd_exit					; ok to continue
;
sd_skip220:
	cpi		R16, DEMO_AVOID_BACKUP
	brne	sd_skip230
; Done with last state?
	lds		R16, demo_s_time
	dec		R16
	sts		demo_s_time, R16		; update
	brne	sd_exit					; no
; Turn around
	ldi		R17, DIR_FWD
	call	pwm_set_right_dir
;
	ldi		R16, 105				; tune for 180 degree turn
	sts		demo_s_time, R16
;
	ldi		R16, DEMO_AVOID_FWD
	sts		demo_state, R16			; next state
	rjmp	sd_exit
; recover
sd_skip230:
	ldi		R16, DEMO_AVOID_FWD
	sts		demo_state, R16			; next state
	rjmp	sd_exit
;
sd_skip30:
	call	pwm_stop_left
	call	pwm_stop_right
; ERROR
sd_exit:
	ret


/*
 * Obstical Check
 *
 * Chech all Sonar range sensors for minimum return ranges.
 *
 * output:	R17		0: Clear .. 1:Obstical
 *
 */
obstical_check:
	lds		r16, range_s_left
	cpi		r16, SONAR_LIMIT
	brge	oc_skip10
	rjmp	oc_block_exit
;
oc_skip10:
	lds		r16, range_s_center
	cpi		r16, SONAR_LIMIT
	brge	oc_skip20
	rjmp	oc_block_exit
;
oc_skip20:
	lds		r16, range_s_right
	cpi		r16, SONAR_LIMIT
	brge	oc_skip30
	rjmp	oc_block_exit
;
oc_skip30:
	clr		r17						; no obsticals
	ret
;
oc_block_exit:
	ldi		r17, 1					; obstical detected
	ret

