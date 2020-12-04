.global gdt_load

reload_sr:
    // update other segment registers
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw %ax, %fs
    movw %ax, %gs
    ret

gdt_load:
    lgdt (%rdi)

    // do a far return to update code segment register
    pushq $0x08
    pushq $reload_sr
    lretq