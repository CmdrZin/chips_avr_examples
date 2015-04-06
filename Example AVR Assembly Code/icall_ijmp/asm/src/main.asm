/*
 * Example Code: ICALL and IJMP
 *
 * org: 04/05/2015
 * rev:
 * auth: Nels "Chip" Pearson
 *
 * Target: Any AVR
 *
 *
 *
 */ 

.nolist
.include "m164pdef.inc"
.list


.ORG	$0000
	rjmp	RESET

.ORG	PCI2addr			; (0x0c) Pin Change Interrupt Request 2
	rjmp	trap_intr		; TMR0 counter compare intr

.ORG	OC0Aaddr
	rjmp	trap_intr		; TMR0 counter compare intr

.ORG	TWIaddr				;
	rjmp	trap_intr
;
.ORG	INT_VECTORS_SIZE	; Don't code in vector table.

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
main:
;
	ldi		ZL, FOO2_INDEX
	call	call_indirect
;
	ldi		ZL, BAR2_INDEX
	call	jump_indirect
;
	rjmp	main

/*
 * For Debug, trap any extrainious interrupts.
 */
trap_intr:
;;;	call	turn_on_led
	rjmp	trap_intr

// Example Code
.include "relative_call_jmp.asm"
