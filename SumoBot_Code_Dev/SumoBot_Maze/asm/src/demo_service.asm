/*
 * Sumo Maze Bot Demo Code
 *
 * org: 10/16/2014
 * rev: 01/21/2015
 * auth: Nels "Chip" Pearson
 *
 * Target: Sumo Maze Bot Board, 20MHz, ATmega164P, Sumo Bot base
 *
 * Dependentcies
 *	floor_det_service.asm
 *	range_service.asm
 *	sys_timers.asm
 *
 * Resources
 * SRAM
 *
 * IO
 *
 */

.equ	DEMO_DELAY_COUNT	= 1		; 10msec

.equ	DEMO_MODE_SQUARE		= 1
.equ	DEMO_MODE_UPBACK		= 2
.equ	DEMO_MODE_AVOID			= 3		; avoid black line, table edge, stairs, drop-offs
.equ	DEMO_MODE_MAZE_LEFT		= 4
.equ	DEMO_MODE_MAZE_RIGHT	= 5

// SQUARE
.equ	DEMO_SQUARE_FWD			= 0		; go forward
.equ	DEMO_SQUARE_TURN90		= 2		; turn right

// AVOID
.equ	DEMO_AVOID_FWD		= 0		; go forward
.equ	DEMO_AVOID_CONTINUE	= 1		; continue forward
.equ	DEMO_AVOID_BACKUP	= 2		; go reverse
.equ	DEMO_AVOID_TURN180	= 3		; go forward

// MAZE WALL FOLLOW




.DSEG
demo_delay:		.BYTE	1
demo_mode:		.BYTE	1		; 0=Square, 1=Up-Back
demo_state:		.BYTE	1		; mode state
demo_s_time:	.BYTE	1		; state time

.CSEG

sumo_demo_init:
	ldi		R16, DEMO_DELAY_COUNT
	sts		demo_delay, R16
;
	ldi		R16, DEMO_MODE_SQUARE
;;;	ldi		R16, DEMO_MODE_AVOID
	sts		demo_mode, R16
;
	ret

/*
 * Some simple state machines to demonstrate to system.
 */
sumo_demo:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	ret									; EXIT..not set
;
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
; check delay
	lds		R16, demo_delay
	dec		R16
	sts		demo_delay, R16
	breq	sd_skip00
	ret									; EXIT..not time
;
sd_skip00:
	ldi		R16, DEMO_DELAY_COUNT		; reset
	sts		demo_delay, R16
// Run service
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
; DB++
;;	call	smb_led1_on
; DB--
	ldi		r18, high(PWM_R_MED_F)
	ldi		r17, low(PWM_R_MED_F)
	call	pwm_set_right

	ldi		r18, high(PWM_L_MED_F)
	ldi		r17, low(PWM_L_MED_F)
	call	pwm_set_left
;
	ldi		R16, 200
	sts		demo_s_time, R16			; run for 2 sec
;
	ldi		R16, DEMO_SQUARE_TURN90
	sts		demo_state, R16					; next state
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
; DB++
;;	call	smb_led1_off
; DB--
	call	pwm_stop_left
;
	ldi		R16, 105
	sts		demo_s_time, R16			; run for 2 sec
;
	ldi		R16, DEMO_SQUARE_FWD
	sts		demo_state, R16					; next state
	rjmp	sd_exit
;
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
	ldi		r16, high(PWM_R_MED_F)
	ldi		r17, low(PWM_R_MED_F)
	call	pwm_set_right

	ldi		r16, high(PWM_L_MED_F)
	ldi		r17, low(PWM_L_MED_F)
	call	pwm_set_left
;
	ldi		r16, DEMO_AVOID_CONTINUE
	sts		demo_state, r16					; next state
	rjmp	sd_exit
;
sd_skip210:
	cpi		R16, DEMO_AVOID_CONTINUE
	brne	sd_skip220
; Check detectors..ALL must be zero
	lds		R16, floor_det_status
;;	cpi		R16, FLOOR_DET_LEFT | FLOOR_DET_CENTER | FLOOR_DET_RIGHT
;;	brne	sd_skip211
	andi	R16, FLOOR_DET_CENTER	; test center only
	brne	sd_skip211
	rjmp	sd_exit					; ok to continue
; Detected BLACK
sd_skip211:
; Backup
	ldi		r16, high(PWM_R_MED_R)
	ldi		r17, low(PWM_R_MED_R)
	call	pwm_set_right
	ldi		r16, high(PWM_L_MED_R)
	ldi		r17, low(PWM_L_MED_R)
	call	pwm_set_left
;
	ldi		R16, 30
	sts		demo_s_time, R16		; run for 1 sec
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
	ldi		r16, high(PWM_R_MED_R)
	ldi		r17, low(PWM_R_MED_R)
	call	pwm_set_right

	ldi		r16, high(PWM_L_MED_F)
	ldi		r17, low(PWM_L_MED_F)
	call	pwm_set_left
;
	ldi		R16, 105
	sts		demo_s_time, R16		; run for 1 sec
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
;
sd_exit:
	ret
