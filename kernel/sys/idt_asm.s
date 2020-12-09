/* stack after interrupt:
  	SS
	RSP
	RFLAGS
	CS
 	RIP
 	ERRCODE?
*/

.global idt_load

.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr30

.extern exc_handler

idt_load:
	lidt (%rdi)
	sti
	ret

.macro exc_noerrcode excnum
	push %rbp
	mov %rsp, %rbp

	push %rax
	push %rdi
	push %rsi
	push %rdx
	push %rcx
	push %r8
	push %r9
	push %r10
	push %r11

	// pass dummy error code
	mov $0, %rdi

	// pass exception number
	mov $\excnum, %rsi

	call exc_handler
	
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rcx
	pop %rdx
	pop %rsi
	pop %rdi
	pop %rax

	pop %rbp
	iretq
.endm

.macro exc_errcode excnum
	// pop the error code
	popq %r12

	push %rbp
	mov %rsp, %rbp

	push %rax
	push %rdi

	// pass the error code
	movq %r12, %rdi

	push %rsi
	push %rdx
	push %rcx
	push %r8
	push %r9
	push %r10
	push %r11

	// pass exception number
	mov $\excnum, %rsi

	call exc_handler
	
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rcx
	pop %rdx
	pop %rsi
	pop %rdi
	pop %rax

	pop %rbp
	iretq
.endm

isr0:	exc_noerrcode 	0
isr1:	exc_noerrcode 	1
isr2:	exc_noerrcode 	2
isr3:	exc_noerrcode 	3
isr4:	exc_noerrcode 	4
isr5:	exc_noerrcode 	5
isr6:	exc_noerrcode 	6
isr7:	exc_noerrcode 	7
isr8:	exc_errcode 	8
isr10:	exc_errcode 	10
isr11:	exc_errcode 	11
isr12: 	exc_errcode 	12
isr13: 	exc_errcode 	13
isr14: 	exc_errcode 	14
isr16: 	exc_noerrcode 	16
isr17: 	exc_errcode 	17
isr18: 	exc_noerrcode 	18
isr19: 	exc_noerrcode 	19
isr20: 	exc_noerrcode 	20
isr30: 	exc_errcode 	30
