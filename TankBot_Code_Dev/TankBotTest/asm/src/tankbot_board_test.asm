/*
 * TankBot Board Test Utilities
 *
 * org: 11/19/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: TankBot Board, 20MHz, ATmega164P
 *
 * Dependentcies:
 *   sys_timers.asm
 *
 */

.equ		TB_STATUS_LED	=	PORTB0


.DSEG
tb_delay:		.BYTE	1
tb_buffer:		.BYTE	4
tb_count:		.BYTE	1


.CSEG
/*
 * Test RS-232 Serial
 *
 */
tb_serial:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	ret									; EXIT..not set
;
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrupt
; check delay
	lds		r16, tb_delay
	dec		r16
	sts		tb_delay, r16
	breq	tbs_skip00
	ret									; EXIT..not time
tbs_skip00:
	ldi		r16, 10						; 100ms rate
	sts		tb_delay, r16
; Send banner
	call	tb_send_banner_serial
; Send a character
	ldi		r17, 'C'
	call	serial_send_byte
; Check for input
	call	serial_recv_byte
	tst		r18
	brne	tbs_exit
	call	serial_send_byte			; echo back
;
tbs_exit:
	ret

/*
 * Send Text Banner through Serial Port
 */
tb_send_banner_serial:
	ldi		ZL, LOW(TB_TEXT_BANNER<<1)
	ldi		ZH, HIGH(TB_TEXT_BANNER<<1)
tsbs_loop00:
	lpm		r17, Z+
	tst		r17
	breq	tsbs_exit
	call	serial_send_byte
	rjmp	tsbs_loop00
;
tsbs_exit:
	ret

// NULL terminated text.
TB_TEXT_BANNER:
.db		'T','A','N','K',' ','B','O','T',0x0A,0x0D,0,0


/*
 * Test logger out to I2C Slave
 *
 */
tb_logger:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	ret									; EXIT..not set
;
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrupt
; check delay
	lds		r16, tb_delay
	dec		r16
	sts		tb_delay, r16
	breq	tbl_skip00
	ret									; EXIT..not time
tbl_skip00:
	ldi		r16, 205
	sts		tb_delay, r16
; Send a message
	call	logger_clear_buffer
	ldi		ZL, LOW(TANK_BOT_ALIVE)
	ldi		ZH, HIGH(TANK_BOT_ALIVE)
	call	logger_append_flash_text
;
;	ldi		ZL, LOW(LOG_TEXT_SPACE)
;	ldi		ZH, HIGH(LOG_TEXT_SPACE)
;	call	logger_append_flash_text
;
;	ldi		r16, 'Z'
;	sts		tb_buffer, r16
;	ldi		ZL, LOW(tb_buffer)
;	ldi		ZH, HIGH(tb_buffer)
;	ldi		r17, 1
;	call	logger_append_sram_text
;
	ldi		ZL, LOW(LOG_TEXT_SPACE)
	ldi		ZH, HIGH(LOG_TEXT_SPACE)
	call	logger_append_flash_text
;
	lds		r16, tb_count
	andi	r16, 0x03				; limit range to 0-3
; switch(tb_count)
	cpi		r16, 0
	brne	tl_skip10
; Left Front
	mov		r17, r16
	call	logger_append_byte_text
;
	ldi		ZL, LOW(LOG_TEXT_SPACE)
	ldi		ZH, HIGH(LOG_TEXT_SPACE)
	call	logger_append_flash_text
;
	lds		r17, range_ir_leftFront
	call	logger_append_byte_text
;
	rjmp	tl_exit
;
tl_skip10:
	cpi		r16, 1
	brne	tl_skip20
; Left Rear
	mov		r17, r16
	call	logger_append_byte_text
;
	ldi		ZL, LOW(LOG_TEXT_SPACE)
	ldi		ZH, HIGH(LOG_TEXT_SPACE)
	call	logger_append_flash_text
;
	lds		r17, range_ir_leftRear
	call	logger_append_byte_text
;
	rjmp	tl_exit
;
tl_skip20:
	cpi		r16, 2
	brne	tl_skip30
; Right Rear
	mov		r17, r16
	call	logger_append_byte_text
;
	ldi		ZL, LOW(LOG_TEXT_SPACE)
	ldi		ZH, HIGH(LOG_TEXT_SPACE)
	call	logger_append_flash_text
;
	lds		r17, range_ir_rightRear
	call	logger_append_byte_text
;
	rjmp	tl_exit
;
tl_skip30:
; Right Front
	mov		r17, r16
	call	logger_append_byte_text
;
	ldi		ZL, LOW(LOG_TEXT_SPACE)
	ldi		ZH, HIGH(LOG_TEXT_SPACE)
	call	logger_append_flash_text
;
	lds		r17, range_ir_rightFront
	call	logger_append_byte_text
;
tl_exit:
	lds		r16, tb_count
	inc		r16
	sts		tb_count, r16
;
	call	logger_send
	ret

.equ	IR_LIMIT = 0x80		; ~10cm
/*
 * Display IR range limit on LEDs
 * Do not uses with tb_sonar_range_leds
 *
 */
tb_ir_range_leds:
	lds		r16, range_ir_leftFront
	cpi		r16, IR_LIMIT
	brlo	tirl_skip01				; unsigned test <
	call	tb_led1_on
	rjmp	tirl_skip10
tirl_skip01:
	call	tb_led1_off
;
tirl_skip10:
	lds		r16, range_ir_leftRear
	cpi		r16, IR_LIMIT
	brlo	tirl_skip11
	call	tb_led2_on
	rjmp	tirl_skip20
tirl_skip11:
	call	tb_led2_off
;
tirl_skip20:
	lds		r16, range_ir_rightFront
	cpi		r16, IR_LIMIT
	brlo	tirl_skip21
	call	tb_led3_on
	rjmp	tirl_skip30
tirl_skip21:
	call	tb_led3_off
;
tirl_skip30:
	lds		r16, range_ir_rightRear
	cpi		r16, IR_LIMIT
	brlo	tirl_skip31
	call	tb_led4_on
	rjmp	tirl_skip40
tirl_skip31:
	call	tb_led4_off
;
tirl_skip40:
	ret


.equ	SONAR_LIMIT = 50		; 50cm
/*
 * Display Sonar range limit on LEDs
 * Do not uses with tb_ir_range_leds
 *
 */
tb_sonar_range_leds:
	lds		r16, range_s_left
	cpi		r16, SONAR_LIMIT
	brsh	tsrl_skip01
	sbi		PORTD, PWM_A1_RIGHT				; ON
	rjmp	tsrl_skip10
tsrl_skip01:
	cbi		PORTD, PWM_A1_RIGHT				; OFF
;
tsrl_skip10:
	lds		r16, range_s_center
	cpi		r16, SONAR_LIMIT
	brsh	tsrl_skip11
	sbi		PORTD, PWM_A2_RIGHT				; ON
	rjmp	tsrl_skip20
tsrl_skip11:
	cbi		PORTD, PWM_A2_RIGHT				; OFF
;
tsrl_skip20:
	lds		r16, range_s_right
	cpi		r16, SONAR_LIMIT
	brsh	tsrl_skip21
	sbi		PORTD, PWM_B1_LEFT				; ON
	rjmp	tsrl_skip30
tsrl_skip21:
	cbi		PORTD, PWM_B1_LEFT				; OFF
;
tsrl_skip30:
	ret


/*
 * Cycle LEDs
 */
tbtest_leds:
	sbi		DDRD, PWM_A2_RIGHT		; Set LED IO to output
	sbi		DDRD, PWM_A1_RIGHT
	sbi		DDRD, PWM_B2_LEFT
	sbi		DDRD, PWM_B1_LEFT
;
	sbi		DDRB, TB_STATUS_LED
;
	ldi		r16, 20
tbl_mloop:
;
tbl_loop00:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop00
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_led1_on
tbl_loop01:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop01
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_led2_on
tbl_loop02:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop02
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_led3_on
tbl_loop03:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop03
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_led4_on
tbl_loop04:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop04
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_status_on
;
tbl_loop10:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop10
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_led1_off
tbl_loop11:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop11
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_led2_off
tbl_loop12:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop12
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_led3_off
tbl_loop13:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop13
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_led4_off
tbl_loop14:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop14
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	call	tb_status_off
;
	dec		r16
	brne	tbl_mloop
;
	ret


/*
 * LED control functions.
 */
tb_led1_on:
	sbi		PORTD, PWM_B2_LEFT				; D2 ON
	ret

tb_led1_off:
	cbi		PORTD, PWM_B2_LEFT				; D2 OFF
	ret

tb_led2_on:
	sbi		PORTD, PWM_B1_LEFT				; D3 ON
	ret

tb_led2_off:
	cbi		PORTD, PWM_B1_LEFT				; D3 OFF
	ret

tb_led3_on:
	sbi		PORTD, PWM_A1_RIGHT				; D4 ON
	ret

tb_led3_off:
	cbi		PORTD, PWM_A1_RIGHT				; D4 OFF
	ret

tb_led4_on:
	sbi		PORTD, PWM_A2_RIGHT				; D5 ON
	ret

tb_led4_off:
	cbi		PORTD, PWM_A2_RIGHT				; D5 OFF
	ret

tb_status_on:
	sbi		PORTB, TB_STATUS_LED			; D6 ON
	ret

tb_status_off:
	cbi		PORTB, TB_STATUS_LED			; D6 OFF
	ret
