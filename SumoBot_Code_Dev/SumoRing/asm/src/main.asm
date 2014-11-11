/*
 * Sumo Bot Test Code Project
 *
 * org: 10/13/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: LCD_CDM-116100 Demo Board, 8MHz, ATmega328P
 *
 * Resources
 * SRAM
 *
 * IO
 *
 */ 

.nolist
.include "m328pdef.inc"
.list



.ORG	$0000
		rjmp	RESET

.ORG	OC1Aaddr				; 0x16 Timer/Counter1 Compare Match A
		rjmp	pwm_tmr1A_intr

.ORG	OC1Baddr				; 0x18 Timer/Counter1 Compare Match B
		rjmp	pwm_tmr1B_intr

.ORG	OC0Aaddr
		rjmp	st_tmr0_intr	; TMR0 counter compare intr

.ORG	TWIaddr
;;;		rjmp	i2c_intr		; 2-wire Serial Interface

	reti

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
	call	init_led_io
	call	st_init_tmr0
	call	lcd_init			; set up LCD
	call	lcd_text_init
	call	pwm_init
	call	sumo_demo_init
	call	floor_det_init
;;	call	range_service_init
;;	call	adc_init_hdwr
;;	call	stepper_init
;;	call	i2c_init_slave		; init I2C interface as a Slave
;;	call	i2c_slave_init		; enable Slave
;
	sei							; enable intr
;
	clr		R17
	call	lcd_set_ddram_adrs
	ldi		ZL, low(lcd_bt_text00<<1)
	ldi		ZH, high(lcd_bt_text00<<1)
	call	out_flash_banner
;
	ldi		R17, 0x40
	call	lcd_set_ddram_adrs
	ldi		ZL, low(lcd_bt_hellow<<1)
	ldi		ZH, high(lcd_bt_hellow<<1)
	call	out_flash_banner
;
; For debug ++
	ldi		ZL, low(lcd_clr_text<<1)
	ldi		ZH, high(lcd_clr_text<<1)
	ldi		R17, 5
	call	lcd_set_flash_text
; --
; reset to begining
	call	lcd_return_home
;
	call	turn_off_green
	call	turn_off_red
	call	turn_off_yellow
;
	call	turn_on_red
	clr		R16
tst_loop00:
	clr		R17
tst_loop01:
	dec		R17
	brne	tst_loop01
	dec		R16
	brne	tst_loop00
	call	turn_off_red
;
main_m:

;
m_skip00:
;;;	call	lcd_text_service
;
m_skip01:
	call	sumo_demo
;
	call	floor_det_service
;;;	call	range_service
;;;	call	stepper_service
;;;	call	i2c_slave_service
;
	rjmp	main_m

// Bring in timmer support
.include "sys_timers.asm"
// LCD CDM-16100 Driver
.include "lcd_cdm_16100_4bit.asm"
// LCD CDM-16100 Text Service
.include "lcd_text_service.asm"
// LCD CDM-16100 Ddmo Board LEDs
.include "lcd_demo_leds.asm"
// PWM Servo Lib
.include "pwm_servo_lib.asm"
// Demo Code
.include "demo_service.asm"
// Floor Detector Code
.include "floor_det_service.asm"
// Optical Range Service
.include "range_service.asm"
// ADC Utilities
.include "adc_util_triggered.asm"
// Conversion Utilities
.include "conversion_util.asm"
// Stepper Motor
.include "stepper_motor.asm"
// I2C Slave code
.include "i2c_slave.asm"
// I2C Slave Service
.include "i2c_slave_service.asm"
