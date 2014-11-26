/*
 * Tank Bot Test Code Project
 *
 * org: 11/13/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Tank Bot Demo Board, 20MHz, ATmega164P
 *
 *
 */ 

.nolist
.include "m164pdef.inc"
.list


.ORG	$0000
		rjmp	RESET

.ORG	OC2Aaddr				; 0x12 Timer/Counter2 Compare Match A
		rjmp	st_tmr2A_intr

.ORG	OC1Aaddr				; 0x16 Timer/Counter1 Compare Match A
		rjmp	pwm_tmr1A_intr

.ORG	OC1Baddr				; 0x18 Timer/Counter1 Compare Match B
		rjmp	pwm_tmr1B_intr

.ORG	OC0Aaddr
		rjmp	st_tmr0_intr	; TMR0 counter compare intr


.ORG	INT_VECTORS_SIZE		; Skip over the rest of them.

.CSEG
RESET:
; setup SP
	ldi		R16, LOW(RAMEND)
	out		spl, R16
	ldi		R16, HIGH(RAMEND)
	out		sph, R16
; JTAG disable
	ldi		R16, $80
	out		MCUCR, R16
	out		MCUCR, R16
;
	call	st_init_tmr0
	call	st_init_tmr1
	call	st_init_tmr2
;;	call	adc_init_hdwr
	call	pwm_dc_init
;;	call	tank_demo_init
;;	call	range_ir_service_init
;;	call	range_s_service_init
;
	sei							; enable intr
;
; TEST++
	ldi		r17, DIR_FWD
	call	pwm_set_right_dir
	ldi		r17, PWM_R_SLOW
	call	pwm_set_right
;
	ldi		r17, DIR_FWD
	call	pwm_set_left_dir
	ldi		r17, PWM_L_MED
	call	pwm_set_left
; TEST--
;
main_m:
;
m_skip00:
	dec		r3
	brne	m_skip00
;
m_skip01:
;;;	call	tank_demo
;
;	call	tbtest_leds
;
;;;	call	range_ir_service
;
	call	range_sonar_service
;
	rjmp	main_m

// Bring in timmer support
.include "sys_timers.asm"
// PWM DC Motor Lib
.include "pwm_dc_motor_lib.asm"
// Demo Code
.include "demo_service.asm"
// Optical Range Service
.include "range_ir_service.asm"
// ADC Utilities
.include "adc_util_triggered.asm"
// Sonar Range Service
.include "range_sonar_service.asm"
// Logger Support
.include "logger_i2c_out.asm"
// Conversion Utilities
.include "conversion_util.asm"
// Board Test
.include "tankbot_board_test.asm"
