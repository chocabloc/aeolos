#include "panic.h"
#include "klog.h"
#include "lock.h"
#include "dev/term/term.h"
#include "symbols.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

extern void kernel_start;
extern void kernel_end;

static spinlock_t panic_lock;

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
    for (int i = 0;; i++) {
        klog_printf(" \t%d: ", i);
        uint64_t func_addr = *(rbp_val + 1);
        const char* func_name = symtab_get_func(func_addr);
        if (!func_name) {
            klog_printf("\t<Unknown Function>");
            break;
        }
        klog_printf("\t%x (%s)\n", func_addr, func_name);
        rbp_val = (uint64_t*)*rbp_val;
    }
}

__attribute__((noreturn)) void kernel_panic(const char* s, ...)
{
    asm volatile("cli");
    spinlock_take(&panic_lock);

    // show on screen if terminal is ready
    if (term_isready())
        klog_show();

    term_setfgcolor(TERM_COLOR_RED);
    klog_puts("[PANIC] ");
    term_setfgcolor(TERM_COLOR_WHITE);

    va_list args;
    va_start(args, s);
    klog_vprintf(s, args);
    va_end(args);

    do_stacktrace();

    while (true)
        asm volatile("hlt");
}
