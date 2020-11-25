#include "kconio.h"
#include "drivers/fbcon/fbcon.h"
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
                fbcon_putchar('%');
                break;

            case 'd':
                fbcon_putint(va_arg(args, int));
                break;

            case 'x':
                fbcon_puthex(va_arg(args, uint64_t));
                break;

            case 's':
                fbcon_puts(va_arg(args, const char*));
                break;
            }
            i++;
        } break;

        default:
            fbcon_putchar(s[i]);
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
    fbcon_puts(s);
    return EXIT_SUCCESS;
}

int kputchar(int i)
{
    fbcon_putchar((uint8_t)i);
    return EXIT_SUCCESS;
}

int kdbg_ok(const char* s, ...)
{
    fbcon_setfgcolor(0x66ff66);
    fbcon_puts("[OKAY] ");
    fbcon_setfgcolor(FBCON_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    int returnval = kvprintf(s, args);
    va_end(args);
    return returnval;
}

int kdbg_info(const char* s, ...)
{
    fbcon_setfgcolor(0x6666ff);
    fbcon_puts("[INFO] ");
    fbcon_setfgcolor(FBCON_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    int returnval = kvprintf(s, args);
    va_end(args);
    return returnval;
}
