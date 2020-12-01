#include "kconio.h"
#include "dev/term/term.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int kvprintf(const char* s, va_list args)
{
    for (size_t i = 0; s[i] != '\0'; i++) {
        switch (s[i]) {
        case '%': {
            switch (s[i + 1]) {
            case '%':
                term_putchar('%');
                break;

            case 'd':
                term_putint(va_arg(args, int));
                break;

            case 'x':
                term_puthex(va_arg(args, uint64_t));
                break;

            case 's':
                term_puts(va_arg(args, const char*));
                break;
            }
            i++;
        } break;

        default:
            term_putchar(s[i]);
        }
    }
    return EXIT_SUCCESS;
}

int kprintf(const char* s, ...)
{
    va_list args;
    va_start(args, s);
    int returnval = kvprintf(s, args);
    va_end(args);
    return returnval;
}

int kputs(const char* s)
{
    term_puts(s);
    return EXIT_SUCCESS;
}

int kputchar(int i)
{
    term_putchar((uint8_t)i);
    return EXIT_SUCCESS;
}

int kdbg_ok(const char* s, ...)
{
    term_setfgcolor(0x66ff66);
    term_puts("[OKAY] ");
    term_setfgcolor(TERM_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    int returnval = kvprintf(s, args);
    va_end(args);
    return returnval;
}

int kdbg_info(const char* s, ...)
{
    term_setfgcolor(0x6666ff);
    term_puts("[INFO] ");
    term_setfgcolor(TERM_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    int returnval = kvprintf(s, args);
    va_end(args);
    return returnval;
}

int kdbg_err(const char* s, ...)
{
    term_setfgcolor(0xff6666);
    term_puts("[ERROR] ");
    term_setfgcolor(TERM_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    int returnval = kvprintf(s, args);
    va_end(args);
    return returnval;
}
