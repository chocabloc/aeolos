#include "kconio.h"
#include "dev/term/term.h"
#include <stddef.h>

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
    return 0;
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
    return 0;
}

int kputsn(const char* s, uint64_t len)
{
    for (uint64_t i = 0; i < len; i++)
        term_putchar(s[i]);
    return 0;
}

int kputchar(int i)
{
    term_putchar((uint8_t)i);
    return 0;
}

int kdbg_ok(const char* s, ...)
{
    term_setfgcolor(TERM_COLOR_LTGREEN);
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
    term_setfgcolor(TERM_COLOR_LTBLUE);
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
    term_setfgcolor(TERM_COLOR_LTRED);
    term_puts("[ERROR] ");
    term_setfgcolor(TERM_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    int returnval = kvprintf(s, args);
    va_end(args);
    return returnval;
}

int kdbg_warn(const char* s, ...)
{
    term_setfgcolor(TERM_COLOR_ORANGE);
    term_puts("[WARNING] ");
    term_setfgcolor(TERM_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    int returnval = kvprintf(s, args);
    va_end(args);
    return returnval;
}
