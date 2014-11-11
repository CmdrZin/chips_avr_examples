/*
 * I2C Master Project
 *
 * org: 9/03/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: I2C Demo Board w/ display and keypad I/O, 20MHz
 *
 * Resources
 * SRAM
 *	i2c_buff:	.BYTE	FIFO_SIZE(18)	; Reserve N bytes in SRAM
 *
 * IO
 *	GPIOR0.GPIOR00		display tic1ms flag
 *	GPIOR0.GPIOR01		key scan tic1ms flag
 *
 */ 

.nolist
.include "m164pdef.inc"
.list



.ORG	$0000
		rjmp	RESET

.ORG	OC0Aaddr
		rjmp	st_tmr0_intr	; TMR0 counter compare intr

.ORG	TWIaddr
		rjmp	i2c_intr		; 2-wire Serial Interface

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
		call	st_init_tmr0		; set up interrupt
		call	i2c_init_master
		call	dwk_init
; Default mode
	ldi		R16, MS_MODE_W_RD_ECHO
	sts		master_state, R16
;
		sei							; enable intr
		rjmp	main_m
;
main_m:
		call	display_kp_service
;
m_skip00:
		call	master_service
;
m_skip01:
;
		rjmp	main_m


// Bring in I2C driver code
.include "i2c_master.asm"
// Bring in timmer support
.include "sys_timers.asm"
// Bring in display and key pad code
.include "disp_w_keypad.asm"
// Bring in led_switch code
.include "led_switch.asm"
// Bring in Master Message support
.include "master_manager.asm"
// Bring in Utilites
.include "utilities.asm"
