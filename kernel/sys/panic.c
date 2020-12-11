#include "panic.h"
#include "dev/term/term.h"
#include "kconio.h"
#include "symbols.h"
#include <stdarg.h>
#include <stdbool.h>

extern void kernel_start;
extern void kernel_end;

// get function name from address, using the symbol table
static const char* symtab_get_func(uint64_t addr)
{
    for (int i = 0; _kernel_symtab[i].addr < UINT64_MAX; i++)
        if (_kernel_symtab[i].addr < addr && _kernel_symtab[i + 1].addr >= addr)
            return _kernel_symtab[i].name;

    return "Unknown Function";
}

static void do_stacktrace()
{
    // get register %rbp
    uint64_t* rbp_val = 0;
    asm volatile("movq %%rbp, %0"
                 : "=rm"(rbp_val));

    kprintf("\nStack Trace:\n");
    for (int i = 0;; i++) {
        kprintf(" \t%d: ", i);
        if ((void*)rbp_val < &kernel_start || (void*)rbp_val >= &kernel_end) {
            kprintf("\t<Invalid Base Pointer>");
            break;
        }
        uint64_t func_addr = *(rbp_val + 1);
        kprintf("\t%x (%s)\n", func_addr, symtab_get_func(func_addr));
        rbp_val = (uint64_t*)*rbp_val;
    }
}

__attribute__((noreturn)) void kernel_panic(const char* s, ...)
{
    term_setfgcolor(TERM_COLOR_RED);
    term_puts("[PANIC] ");

    term_setfgcolor(TERM_COLOR_WHITE);
    va_list args;
    va_start(args, s);
    kvprintf(s, args);
    va_end(args);

    do_stacktrace();

    asm volatile("cli");
    while (true)
        asm volatile("hlt");
}
