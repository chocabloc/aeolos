#include "panic.h"
#include "apic/timer.h"
#include "dev/term/term.h"
#include "klog.h"
#include "lock.h"
#include "mm/mm.h"
#include "symbols.h"
#include "sys/pit.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

static spinlock_t panic_lock;

__attribute__((interrupt)) static void halt(void* v __attribute__((unused)))
{
    asm volatile("cli");
    while (true)
        asm volatile("hlt");
}

// get function name from address, using the symbol table
static const char* symtab_get_func(uint64_t addr)
{
    for (int i = 0; _kernel_symtab[i].addr < UINT64_MAX; i++)
        if (_kernel_symtab[i].addr < addr && _kernel_symtab[i + 1].addr >= addr)
            return _kernel_symtab[i].name;

    return NULL;
}

static void do_stacktrace()
{
    // get register %rbp
    uint64_t* rbp_val = 0;
    asm volatile("movq %%rbp, %0"
                 : "=rm"(rbp_val));

    klog_printf("\nStack Trace:\n");
    if ((uint64_t)rbp_val <= HIGHERHALF_OFFSET) {
        klog_printf("\n \t<optimised out>");
        return;
    }
    for (int i = 0;; i++) {
        klog_printf(" \t%d: ", i);
        uint64_t func_addr = *(rbp_val + 1);
        const char* func_name = symtab_get_func(func_addr);
        klog_printf("\t%x", func_addr);
        if (!func_name) {
            klog_printf(" (Unknown Function)");
            break;
        }
        klog_printf(" (%s)\n", func_name);
        rbp_val = (uint64_t*)*rbp_val;
    }
}

__attribute__((noreturn)) void kernel_panic(const char* s, ...)
{
    asm volatile("cli");
    spinlock_take(&panic_lock);

    // stop other cores
    apic_timer_set_handler(halt);

    // wait for some time for cores to stop
    pit_wait(10);

    // now print error information
    klog_puts("\033[31m[PANIC] \033[0m");
    va_list args;
    va_start(args, s);
    klog_vprintf(s, args);
    va_end(args);
    do_stacktrace();
    klog_show_urgent();

    // halt this core
    while (true)
        asm volatile("hlt");
}
