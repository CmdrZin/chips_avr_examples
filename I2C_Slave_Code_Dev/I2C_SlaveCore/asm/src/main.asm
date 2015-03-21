/*
 * I2C Slave Library Project
 *
 * org: 10/03/2014
 * rev: 03/20/2015
 * auth: Nels "Chip" Pearson
 *
 * Target: TankBot Board (ATmega164P) or I2C Demo Board
 *
 *
 * NOTE: Slave Adrs: 0x23 (set in i2c_slave2.asm)
 *
 */ 

.nolist
.include "m164pdef.inc"
;;;.include "m328pdef.inc"
.list


.ORG	$0000
	rjmp	RESET

.ORG	OC0Aaddr
	rjmp	st_tmr0_intr	; TMR0 counter compare intr

.ORG	TWIaddr					; (0x34)
	rjmp	i2c_slave_isr	; 2-wire Serial Interface
;
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
	call	st_init_tmr0		; init System Timer (Timer0)
	call	i2c_init_slave		; init I2C interface as a Slave
	call	slave_core_init		; enable Slave and services.
	call	led_init			; set up board LED
	call	dwk_init			; set up dual 7-seg display and keypad IO
;
	sei							; enable intr
; TEST ++
	ldi		r16, 2
	sts		led_buff, r16
;
	ldi		r17, 0xA
	call	dsp_dual_store
	ldi		r17, 0x5
	call	dsp_dual_store
;
;;;	ldi		r17, CORE_GET_ID
;;;	call	core_command_processor
; TEST --
;
main:
;
m_skip00:
	call	slave_core_service	; I2C command service
;
	call	display_kp_service	; dual 7-seg display service
;
;;;	call	switch_service		; button and LDE service	Using LED for DEBUG timing.
;
	rjmp	main

/*
 * For Debug, trap any extrainious interrupts.
 */
trap_intr:
	call	turn_on_led
	rjmp	trap_intr

// System Timer support
.include "sys_timers.asm"
// I2C Slave code
.include "i2c_slave2.asm"
// FIFO support
.include "fifo_lib.asm"
// I2C Core Service
.include "i2c_slave_core.asm"
// I2C Demo Board Service
.include "i2c_slave_i2cDemo.asm"
// 7Seg Display and KeyPad Service
.include "disp_w_keypad.asm"
// Single Buttion and LED Service
.include "led_switch.asm"
