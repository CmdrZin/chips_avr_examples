/*
 * Sumo Maze Bot Test Code Project
 *
 * org: 01/20/2015
 * auth: Nels "Chip" Pearson
 *
 * Target: Sumo Maze Bot Board, 20MHz, ATmega164P
 *
 *
 */ 

.nolist
.include "m164pdef.inc"
.list


.ORG	$0000
	rjmp	RESET
.ORG	$0002
	rjmp	trap_intr
.ORG	$0004
	rjmp	trap_intr
.ORG	$0006
	rjmp	trap_intr
.ORG	$0008
	rjmp	trap_intr
.ORG	$000A
	rjmp	trap_intr

.ORG	PCI2addr				; 0x0c Pin Change Interrupt Request 2
	rjmp	trap_intr
.ORG	$000E
	rjmp	trap_intr
.ORG	$0010
	rjmp	trap_intr

.ORG	OC2Aaddr				; 0x12 Timer/Counter2 Compare Match A
	rjmp	trap_intr
.ORG	$0014
	rjmp	trap_intr
.ORG	$0016
	rjmp	trap_intr
.ORG	$0018
	rjmp	trap_intr

.ORG	OC1Aaddr				; 0x1a Timer/Counter1 Compare Match A
	rjmp	trap_intr

.ORG	OC1Baddr				; 0x1c Timer/Counter1 Compare Match B
	rjmp	trap_intr
.ORG	OVF1addr
	rjmp	smb_timer1_intr		; 0x1e Timer/Counter1 Overflow

.ORG	OC0Aaddr				; 0x20 Timer/Counter0 Compare Match A
	rjmp	st_tmr0_intr
.ORG	$0022
	rjmp	trap_intr
.ORG	$0024
	rjmp	trap_intr
.ORG	$0026
	rjmp	trap_intr
.ORG	$0028
	rjmp	trap_intr
.ORG	$002A
	rjmp	trap_intr
.ORG	$002C
	rjmp	trap_intr
.ORG	$002E
	rjmp	trap_intr
.ORG	$0030
	rjmp	trap_intr
.ORG	$0032
	rjmp	trap_intr

.ORG	TWIaddr					; 0x34 2-wire Serial Interface
	rjmp	trap_intr
.ORG	$0036
	rjmp	trap_intr
.ORG	$0038
	rjmp	trap_intr
.ORG	$003A
	rjmp	trap_intr
.ORG	$003C
	rjmp	trap_intr


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
	call	floor_det_init
	call	adc_init_hdwr
	call	range_service_init
	call	pwm_init
	call	sumo_demo_init
;
	sei							; enable intr
;
	call	smb_led1_on
	call	smb_led1_off
;
main_m:
;
	call	floor_det_service
;
	call	range_service
;
	call	sumo_demo
;;;	call	smb_set_pwm
;
m_skip01:
	rjmp	main_m

trap_intr:
;;	call	tb_led3_on
	rjmp	trap_intr

// Bring in timmer support
.include "sys_timers.asm"
// ADC support
.include "adc_triggered.asm"
// Floor sensors (line follower)
.include "floor_det_service.asm"
// Side Range sensor support
.include "range_service.asm"
// PWM support
.include "pwm_servo_lib.asm"
// DEMO service
.include "demo_service.asm"
// RS-232 Serial support
.include "serial_lib.asm"
// FIFO support
.include "fifo_lib.asm"
// Board test
.include "sumomaze_boardtest.asm"

