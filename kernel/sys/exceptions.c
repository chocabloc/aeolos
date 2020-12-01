#include "kconio.h"
#include <stdbool.h>
#include <stdint.h>

static char* exceptions[] = {
    [0] = "Division by Zero",
    [1] = "Debug",
    [2] = "Non Maskable Interrupt",
    [3] = "Breakpoint",
    [4] = "Overflow",
    [5] = "Bound Range Exceeded",
    [6] = "Invalid opcode",
    [7] = "Device not available",
    [8] = "Double Fault",
    [10] = "Invalid TSS",
    [11] = "Segment not present",
    [12] = "Stack Exception",
    [13] = "General Protection fault",
    [14] = "Page fault",
    [16] = "x87 Floating Point Exception",
    [17] = "Alignment check",
    [18] = "Machine check",
    [19] = "SIMD floating point Exception",
    [20] = "Virtualization Exception",
    [30] = "Security Exception"
};

void exc_handler(uint64_t errcode, uint64_t excno)
{
    kdbg_err("Fatal Exception Occured: %s. Error Code: %d. Halting...\n", exceptions[excno], errcode);

    while (true)
        ;
}
