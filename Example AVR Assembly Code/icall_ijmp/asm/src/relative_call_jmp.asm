/*
 * Example Code: ICALL and IJMP
 *
 * ICALL - Indirect Call to Subroutine
 *
 * IJMP - Indirect Jump
 */


// NOTE: Table can not cross a 256 address boundry.
// If after the .equ, table lable is 0. MUST be before .equ reference.
// If you move this to below the .equ, the LDI in main() will load a 2 into ZL.
// Apperently the compiler can not do two forward references.
call_table:
.dw	foo1
.dw	foo2

// NOTE: Table can not cross a 256 address boundry.
jump_table:
.dw	bar1
.dw	bar2

// MUST be placed AFTER table definition.
.equ	FOO1_INDEX	= low(call_table<<1)
.equ	FOO2_INDEX	= low(call_table<<1)+2

.equ	BAR1_INDEX	= low(jump_table<<1)
.equ	BAR2_INDEX	= low(jump_table<<1)+2


foo1:
	clr		r0
	ret

foo2:
	clr		r1
	ret

bar1:
	clr		r0
	ret

bar2:
	clr		r1
	ret

/* Pass in index ZL from equ list. */
call_indirect:
	ldi		ZH, high(call_table<<1)
; get function address
	lpm		r0, Z+
	lpm		ZH, Z
	mov		ZL, r0
;
	icall
	ret

/* Pass in index ZL from equ list. */
jump_indirect:
	ldi		ZH, high(jump_table<<1)
; get function address
	lpm		r0, Z+
	lpm		ZH, Z
	mov		ZL, r0
;
	ijmp
; jumped to functions provide the RET.

