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
	rjmp	range_s_intr
.ORG	$000E
	rjmp	trap_intr
.ORG	$0010
	rjmp	trap_intr

.ORG	OC2Aaddr				; 0x12 Timer/Counter2 Compare Match A
	rjmp	st_tmr2A_intr
.ORG	$0014
	rjmp	trap_intr
.ORG	$0016
	rjmp	trap_intr
.ORG	$0018
	rjmp	trap_intr

.ORG	OC1Aaddr				; 0x1a Timer/Counter1 Compare Match A
	rjmp	pwm_tmr1A_intr

.ORG	OC1Baddr				; 0x1c Timer/Counter1 Compare Match B
	rjmp	pwm_tmr1B_intr
.ORG	$001E
	rjmp	trap_intr

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
	rjmp	i2c_intr
.ORG	$0036
	rjmp	trap_intr
.ORG	$0038
	rjmp	trap_intr
.ORG	$003A
	rjmp	trap_intr
.ORG	$003C
	rjmp	trap_intr

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
	call	adc_init_hdwr
	call	pwm_dc_init
;;	call	tank_demo_init
	call	range_ir_service_init
	call	range_s_service_init
	call	i2c_init_master
;
	sei							; enable intr
;
	call	tbtest_leds			; blink LEDs for awhile.
;
main_m:
;
m_skip01:
; use only one of these at a time. NOT with demo. Messes up PWM.
;;	call	tb_ir_range_leds
;;	call	tb_sonar_range_leds
;
	call	tb_logger				; Output a test message every 100ms.
;
;;;	call	tank_demo
;
	call	range_ir_service
;
	call	range_sonar_service
;
;
	rjmp	main_m

trap_intr:
	call	tb_led3_on
	rjmp	trap_intr

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
// I2C Master Support
.include "i2c_master.asm"

