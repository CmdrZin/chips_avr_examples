/*
 * Optocal Range Sensor Service
 *
 * org: 10/20/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Sumo Bot Demo Board, 8MHz w/65ms reset delay,
 *
 * Resources
 * SRAM
 *
 * IO
 *
 */

.equ	RNG_LED_DELAY_COUNT		= 3		; 30ms
.equ	RNG_IDLE_DELAY_COUNT	= 30	; 300ms


.equ	RNG_WAIT_IDLE			= 0
.equ	RNG_WAIT_LEFT			= 1
.equ	RNG_WAIT_RIGHT			= 2


.DSEG
range_left:		.BYTE	1		; 0x00=4cm..0xF0=15cm
range_right:	.BYTE	1
range_state:	.BYTE	1
range_delay:	.BYTE	1		; 30ms nominal

.CSEG


/*
 * Initialize Optical Range Sensor
 *
 */
range_service_init:
	call	range_service_init_io
;
	ldi		R16, RNG_IDLE_DELAY_COUNT
	sts		range_delay, R16
	ldi		R16, RNG_WAIT_IDLE
	sts		range_state, R16
;
	ret

/*
 * Configure IO pins
 */
range_service_init_io:

	ret

/*
 * range_service()
 *
 * Display text on display.
 *
 * input reg:	none
 *
 * output reg:	none
 *
 * resources:	range_left
 *				range_right
 *
 * Two ADC channels for IR distance.
 * Set up ADC to trigger after LED turn-on. 30ms cycle.
 * Toggle between sensors.
 *
 * Process
 * 0. Wait IDLE 300ms then Light Left LED for 30ms
 * 1. Wait 30ms then Sample Left Detector, Light Right LED for 30ms
 * 2. Wait 30ms then Sample Right Detector, Set IDLE delay for 300ms
 *
 */
range_service:
	sbis	GPIOR0, RNG_10MS_TIC	; test 10ms tic
	ret								; EXIT..not set
;
	cbi		GPIOR0, RNG_10MS_TIC	; clear tic10ms flag set by interrup
// Run service
	lds		R16, range_delay
	dec		R16
	sts		range_delay, R16
	brne	rs_exit
; yes
	ldi		R16, RNG_LED_DELAY_COUNT	; most states use LED time
	sts		range_delay, R16
;
	lds		R16, range_state		; get state
; switch(state)
	cpi		R16, RNG_WAIT_IDLE
	brne	rs_skip00
; Leave IDLE
; Turn ON Left Sensor LED
;	cbi		PORTD, RNG_LED_LEFT
; Delay already set for 30ms
	ldi		R16, RNG_WAIT_LEFT		; next state
	sts		range_state, R16
	rjmp	rs_exit
;
rs_skip00:
	cpi		R16, RNG_WAIT_LEFT
	brne	rs_skip10
; Sample Left Det
	ldi		R17, ADC_LEFT_CHAN
	call	adc_trigger				; returns R17.R18..left justified b9:2,b1:0
; Turn OFF Left Sensor LED
;	sbi		PORTD, RNG_LED_LEFT
; DEBUG ++
;;;	call	con_adcLS_ascii			; ADC to HEX..returns X->Text
;;;	ldi		R17, 5					; line 0 Right side.
;;;	call	lcd_set_sram_text
;;;	call	lcd_return_home			; HOME after new text adjusted DGRAM adrs.
; DEBUG --
; Turn ON Right Sensor LED
;	cbi		PORTD, RNG_LED_RIGHT
;
	ldi		R16, RNG_WAIT_RIGHT
	sts		range_state, R16
	rjmp	rs_exit
;
rs_skip10:
	cpi		R16, RNG_WAIT_RIGHT
	brne	rs_skip20
; Sample Right Det
	ldi		R17, ADC_RIGHT_CHAN
	call	adc_trigger
; Set IDLE delay for 300ms
	ldi		R16, RNG_IDLE_DELAY_COUNT
	sts		range_delay, R16
;
rs_skip20:
; set to default
	ldi		R16, RNG_WAIT_IDLE
	sts		range_state, R16
rs_exit:
	ret
