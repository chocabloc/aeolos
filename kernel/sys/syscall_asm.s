.extern syscall_dispatcher

/* syscall entry function
   rdi: syscall index
   rsi: param1
   rdx: param2
*/
.global syscall_entry
syscall_entry:
    push %rbx
    push %rbp
    push %r12
    push %r13
    push %r14
    push %r15
    call syscall_dispatcher
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbp
    pop %rbx
    iretq
