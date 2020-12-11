.extern kmain

.global _start

_start:
	// clear rbp so that stack traces work properly
	xorq %rbp, %rbp
	
	call kmain
	// no returning from here
