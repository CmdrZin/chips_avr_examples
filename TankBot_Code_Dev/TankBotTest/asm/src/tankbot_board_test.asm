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
 * Cycle LEDs
 */
tbtest_leds:
	sbi		PORTD, PWM_A2_RIGHT
	sbi		PORTD, PWM_A1_RIGHT
	sbi		PORTD, PWM_B2_LEFT
	sbi		PORTD, PWM_B1_LEFT
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
	rjmp	tbl_loop00
;
	ret
