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


/*
 * Display IR range limit on LEDs
 * Do not uses with tb_sonar_range_leds
 *
 */
tb_ir_range_leds:
	lds		r16, range_ir_leftFront
	cpi		r16, 0x80
	brge	tirl_skip01
	sbi		PORTD, PWM_A1_RIGHT				; ON
	rjmp	tirl_skip10
tirl_skip01:
	cbi		PORTD, PWM_A1_RIGHT				; OFF
;
tirl_skip10:
	lds		r16, range_ir_leftRear
	cpi		r16, 0x80
	brge	tirl_skip11
	sbi		PORTD, PWM_A2_RIGHT				; ON
	rjmp	tirl_skip20
tirl_skip11:
	cbi		PORTD, PWM_A2_RIGHT				; OFF
;
tirl_skip20:
	lds		r16, range_ir_rightFront
	cpi		r16, 0x80
	brge	tirl_skip21
	sbi		PORTD, PWM_B1_LEFT				; ON
	rjmp	tirl_skip30
tirl_skip21:
	cbi		PORTD, PWM_B1_LEFT				; OFF
;
tirl_skip30:
	lds		r16, range_ir_rightRear
	cpi		r16, 0x80
	brge	tirl_skip31
	sbi		PORTD, PWM_B2_LEFT				; ON
	rjmp	tirl_skip40
tirl_skip31:
	cbi		PORTD, PWM_B2_LEFT				; OFF
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
	brge	tsrl_skip01
	sbi		PORTD, PWM_A1_RIGHT				; ON
	rjmp	tsrl_skip10
tsrl_skip01:
	cbi		PORTD, PWM_A1_RIGHT				; OFF
;
tsrl_skip10:
	lds		r16, range_s_center
	cpi		r16, SONAR_LIMIT
	brge	tsrl_skip11
	sbi		PORTD, PWM_A2_RIGHT				; ON
	rjmp	tsrl_skip20
tsrl_skip11:
	cbi		PORTD, PWM_A2_RIGHT				; OFF
;
tsrl_skip20:
	lds		r16, range_s_right
	cpi		r16, SONAR_LIMIT
	brge	tsrl_skip21
	sbi		PORTD, PWM_B1_LEFT				; ON
	rjmp	tsrl_skip30
tsrl_skip21:
	cbi		PORTD, PWM_B1_LEFT				; OFF
;
tsrl_skip30:
	ret


/*
 * Cycle LEDs 255 times.
 */
tbtest_leds:
	sbi		PORTD, PWM_A2_RIGHT
	sbi		PORTD, PWM_A1_RIGHT
	sbi		PORTD, PWM_B2_LEFT
	sbi		PORTD, PWM_B1_LEFT
;
	clr		r4
tbl_mloop:
;
tbl_loop00:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop00
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	cbi		PORTD, PWM_A2_RIGHT
tbl_loop01:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop01
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	cbi		PORTD, PWM_A1_RIGHT
tbl_loop02:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop02
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	cbi		PORTD, PWM_B2_LEFT
tbl_loop03:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop03
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	sbi		PORTD, PWM_B1_LEFT
;
tbl_loop10:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop10
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	sbi		PORTD, PWM_A2_RIGHT
tbl_loop11:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop11
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	sbi		PORTD, PWM_A1_RIGHT
tbl_loop12:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop12
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	sbi		PORTD, PWM_B2_LEFT
tbl_loop13:
	sbis	GPIOR0, DEMO_10MS_TIC		; test 10ms tic
	rjmp	tbl_loop13
	cbi		GPIOR0, DEMO_10MS_TIC		; clear tic10ms flag set by interrup
	cbi		PORTD, PWM_B1_LEFT
;
	dec		r4
	brne	tbl_mloop
;
	ret
