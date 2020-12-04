#include "panic.h"
#include "dev/term/term.h"
#include "kconio.h"
#include <stdarg.h>
#include <stdbool.h>

__attribute__((noreturn)) void kernel_panic(const char* s, ...)
{
    term_setfgcolor(TERM_COLOR_RED);
    term_puts("[PANIC] ");

    term_setfgcolor(TERM_COLOR_WHITE);
    va_list args;
    va_start(args, s);
    kvprintf(s, args);
    va_end(args);

    asm volatile("cli");
    while (true)
        asm volatile("hlt");
}