/*
 * R2_Comm_Link Project
 *
 * org: 6/22/2014
 * rev: 9/03/2014
 * auth: Nels "Chip" Pearson
 *
 * Resources
 * SRAM
 *	i2c_buff:	.BYTE	FIFO_SIZE(8)	; Reserve N bytes in SRAM
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

.ORG	OC1Baddr
		rjmp	tgu_gen_tone	; Tone generator (8kps)

.ORG	OC0Aaddr
		rjmp	st_tmr0_intr	; TMR0 counter compare intr

.ORG	ADCCaddr
		rjmp	adc_intr

.ORG	TWIaddr
		rjmp	i2c_intr		; 2-wire Serial Interface
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
		call	st_init_tmr0		; set up TMR0interrupt
		call	st_init_tmr1		; set up TMR1 A&B ADC Auto trigger timer
		call	led_bank_init
		call	adc_init_hdwr
		call	goertzel_init
		call	tcomm_init
;
		ldi		R16, S_MODE_NORMAL	; TODO - Move this to init somewhere
		sts		slave_mode, R16
;
		call	i2c_init_slave
		call	i2c_slave_init
;
		sei							; enable intr
; DEBUG
		ldi		R16, 0x96
		sts		led_bank, R16
		ldi		R16, 0x04
		sts		led_cntl, R16
;
main:
		call	led_bank_service
;
		call	tcomm_service
;
		call	slave_service
;
		rjmp	main


// Bring in timmer support
.include "adc_util.asm"
// Bring in timer support
.include "sys_timers.asm"
// Bring in led_bank code
.include "led_bank.asm"
// Bring Goertzel code
.include "goertzel.asm"
// Bring in Tone Generator
.include "sig_generate.asm"
// I2C Support
.include "i2c_slave.asm"
// Slave Manager
.include "slave_manager.asm"
// Tone Manager
.include "tone_manager.asm"
// Utilities
.include "utilities.asm"
