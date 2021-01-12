#include "klog.h"
#include "dev/term/term.h"
#include <stdbool.h>
#include <stddef.h>

// ring buffer for kernel log
static uint8_t log_buff[16384];
static uint16_t log_start = 0;
static uint16_t log_end = 0;

// is the log shown
static bool shown = false;

static void klog_putch(uint8_t i)
{
    log_buff[log_end++] = i;

    if (!log_end || log_start)
        log_start++;

    if (shown)
        term_putchar(i);
}

void klog_show()
{
    shown = true;
}

void klog_hide()
{
    shown = false;
}

void klog_putchar(uint8_t i)
{
    klog_putch(i);
    if (shown)
        term_flush();
}

void klog_putsn(const char* s, uint64_t len)
{
    for (uint64_t i = 0; i < len; i++)
        klog_putch(s[i]);

    if (shown)
        term_flush();
}

void klog_puts(const char* s)
{
    for (uint64_t i = 0; s[i] != '\0'; i++)
        klog_putch(s[i]);

    if (shown)
        term_flush();
}

// print number as 64-bit hex
void klog_puthex(uint64_t n)
{
    klog_puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        uint64_t digit = (n >> i) & 0xF;
        klog_putch((digit <= 9) ? (digit + '0') : (digit - 10 + 'A'));
    }

    if (shown)
        term_flush();
}

// print number
void klog_putint(int n)
{
    if (n == 0)
        klog_putch('0');

    if (n < 0) {
        klog_putch('-');
        n = -n;
    }
    size_t div = 1;
    int temp = n;
    while (temp > 0) {
        temp /= 10;
        div *= 10;
    }
    while (div >= 10) {
        int digit = ((n % div) - (n % (div / 10))) / (div / 10);
        div /= 10;
        klog_putch(digit + '0');
    }

    if (shown)
        term_flush();
}

void klog_vprintf(const char* s, va_list args)
{
    for (size_t i = 0; s[i] != '\0'; i++) {
        switch (s[i]) {
        case '%': {
            switch (s[i + 1]) {
            case '%':
                klog_putch('%');
                break;

            case 'd':
                klog_putint(va_arg(args, int));
                break;

            case 'x':
                klog_puthex(va_arg(args, uint64_t));
                break;

            case 's':
                klog_puts(va_arg(args, const char*));
                break;

            case 'b':
                klog_puts(va_arg(args, int) ? "true" : "false");
                break;
            }
            i++;
        } break;

        default:
            klog_putch(s[i]);
        }
    }

    if (shown)
        term_flush();
}

void klog_printf(const char* s, ...)
{
    va_list args;
    va_start(args, s);
    klog_vprintf(s, args);
    va_end(args);
}

void klog_ok(const char* s, ...)
{
    term_setfgcolor(TERM_COLOR_LTGREEN);
    klog_puts("[OKAY] ");
    term_setfgcolor(TERM_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    klog_vprintf(s, args);
    va_end(args);
}

void klog_info(const char* s, ...)
{
    term_setfgcolor(TERM_COLOR_LTBLUE);
    klog_puts("[INFO] ");
    term_setfgcolor(TERM_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    klog_vprintf(s, args);
    va_end(args);
}

void klog_err(const char* s, ...)
{
    term_setfgcolor(TERM_COLOR_LTRED);
    klog_puts("[ERROR] ");
    term_setfgcolor(TERM_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    klog_vprintf(s, args);
    va_end(args);
}

void klog_warn(const char* s, ...)
{
    term_setfgcolor(TERM_COLOR_ORANGE);
    klog_puts("[WARNING] ");
    term_setfgcolor(TERM_COLOR_GRAY);

    va_list args;
    va_start(args, s);
    klog_vprintf(s, args);
    va_end(args);
}
