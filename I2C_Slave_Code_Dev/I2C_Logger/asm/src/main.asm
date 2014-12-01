/*
 * I2C Slave Library Project
 *
 * org: 10/03/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: R2 Link Board (ATmega164P) or LCD CDM-16100 Display (ATmega328P)
 *
 *
 * Slave Adrs: 0x57
 *
 */ 

.nolist
;;;.include "m164pdef.inc"
.include "m328pdef.inc"
.list


.ORG	$0000
	rjmp	RESET

.ORG	OC0Aaddr				; 0x1c TimerCounter0 Compare Match A
	rjmp	st_tmr0_intr

.ORG	TWIaddr					; 0x30 Two-wire Serial Interface
	rjmp	i2c_intr
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
	call	st_init_tmr0
	call	init_led_io
	call	i2c_init_slave		; init I2C interface as a Slave
	call	lcd_init
	call	lcd_text_init
	call	logger_init
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
	ldi		R17, 6							; line Left(0:4) 0-4, Right(0:4) 5-9
	call	lcd_set_flash_text
; --
; reset to begining
	call	lcd_return_home
;
;
	call	turn_off_green
	call	turn_off_red
	call	turn_off_yellow
;
main:
;
; TWEA = 0 will inhibit the Slave from responding to its address.
/*
; simple I2C test
	lds		r16, i2c_status
	cpi		r16, I2C_STATUS_IDLE
	brne	m_skip00
;
	ldi		R16, 4
	sts		i2c_buffer_out_cnt, R16		; reset count so data will be sent again when requested.
;
 */
 	call	lcd_text_service
 ;
 m_skip00:
	call	logger_check_for_message
;
	rjmp	main


// I2C Slave code
.include "i2c_slave.asm"
// Logger code
.include "logger.asm"
// LCD Display
.include "lcd_cdm_16100_4bit.asm"
// LCD Text Formatting
.include "lcd_text_service.asm"
// System Timers
.include "sys_timers.asm"
// Board Test
.include "lcd_cdm_board_test.asm"
