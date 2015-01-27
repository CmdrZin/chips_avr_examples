/*
 * Sumo Maze Bot Board Test Code
 *
 * org: 01/24/2015
 * rev:
 * auth: Nels "Chip" Pearson
 *
 * Target: Sumo Maze Bot Board, 20MHz, ATmega164P, Sumo Bot base
 *
 * Dependentcies
 *
 * Resources
 * SRAM
 *
 * IO
 *
 */

.equ		SMB_STATUS_LED	=	PORTD2

.DSEG
smb_led1_toggle:	.BYTE	1

.CSEG


/*
 * PWM Test..Disable Demo service when using this function.
 */
smb_set_pwm:
	call	pwm_stop_right
	call	pwm_stop_left
	ret

/*
 * LED control functions.
 */
smb_led1_on:
	sbi		DDRD, SMB_STATUS_LED
	sbi		PORTD, SMB_STATUS_LED
	ret

smb_led1_off:
	cbi		PORTD, SMB_STATUS_LED				; D2 OFF
	ret

/*
 * Timer1 test
 */
smb_timer1_intr:
; Save SREG
	push	r0
	in		r0, SREG
	push	r0
;
	push	r16
;
	lds		r16, smb_led1_toggle
	tst		r16
	breq	smb_ti_skip00
	call	smb_led1_on
	clr		r16
	rjmp	smb_ti_update
;
smb_ti_skip00:
	call	smb_led1_off
	ldi		r16, 1
;
smb_ti_update:
	sts		smb_led1_toggle, r16
;
	pop		r16
; Restore SREG
	pop		r0
	out		SREG, r0
	pop		r0
;
	reti
