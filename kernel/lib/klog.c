#include "klog.h"
#include "dev/serial/serial.h"
#include "dev/term/term.h"
#include "lock.h"
#include "proc/sched/sched.h"
#include "proc/task.h"
#include "sys/hpet.h"
#include <stdbool.h>

// ring buffer for kernel log
static uint8_t log_buff[KLOG_BUFF_LEN];
static uint16_t log_start = 0;
static uint16_t log_end = 0;

// lock to prevent concurrent modification
static lock_t log_lock;

static void putch(uint8_t i)
{
    log_buff[log_end++] = i;
    if (log_end == log_start)
        log_start++;
    serial_send((char)i);
}

static void putsn(const char* s, uint64_t len)
{
    for (uint64_t i = 0; i < len; i++)
        putch(s[i]);
}

static void puts(const char* s)
{
    for (uint64_t i = 0; s[i] != '\0'; i++)
        putch(s[i]);
}

// print number in arbitrary base
static void putnum(uint64_t num, int base, bool is_signed) {
    static char lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                tmpbuf[65] = {0};
    int len = 1;

    // add a minus sign if required
    if ((int64_t)num < 0 && is_signed) {
        putch('-');
        num = -num;
    }

    // extract characters in temp buffer
    do {
        tmpbuf[len++] = lookup[num % base];
        num /= base;
    } while(num);

    // print characters
    while(--len)
        putch(tmpbuf[len]);
}

static void klog_show_helper()
{
    // maximum no of chars that can fit on screen
    uint32_t width = term_getwidth(), height = term_getheight(),
             numchars = width * height;

    /*
     * We don't print all the characters, since that would be terribly
     * slow. Instead, we calculate the number of characters actually
     * visible on screen and print those
     */
    uint32_t visible = 0, used = 0;
    for (uint16_t i = log_end; i != log_start && used < numchars; i--, visible++) {
        if (log_buff[i] == '\n')
            used += (used % width != 0) ? width - (used % width) : 0;
        else if (log_buff[i])
            used += 1;
    }

    // now print the characters
    for (uint16_t i = log_end - visible; i != log_end; i++)
        term_putchar(log_buff[i]);
}

_Noreturn static void klogdisplayd(tid_t tid)
{
    (void)tid;

    while (true) {
        timeval_t begin = hpet_get_nanos();
        term_clear();
        lock_wait(&log_lock);
        klog_show_helper();
        lock_release(&log_lock);
        term_flush();
        timeval_t end = hpet_get_nanos(), delta = end - begin;

        // we want to lock the fps at about 60
        if (delta >= MILLIS_TO_NANOS(16))
            continue;
        sched_sleep(MILLIS_TO_NANOS(16) - delta);
    }
}

static void vprintf(const char* s, va_list args)
{
    for (size_t i = 0; s[i] != '\0'; i++) {
        switch (s[i]) {
        case '%': {
            switch (s[i + 1]) {
            case '%':
                putch('%');
                break;

            case 'd':
                putnum((uint64_t)va_arg(args, int), 10, true);
                break;

            case 'x':
                puts("0x");
                putnum(va_arg(args, uint64_t), 16, false);
                break;

            case 's':
                puts(va_arg(args, const char*));
                break;

            case 'b':
                puts(va_arg(args, int) ? "true" : "false");
                break;
            }
            i++;
        } break;

        default:
            putch(s[i]);
        }
    }
}

void klog_putchar(uint8_t i)
{
    lock_wait(&log_lock);
    putch(i);
    lock_release(&log_lock);
}

void klog_puts(const char* s)
{
    lock_wait(&log_lock);
    puts(s);
    lock_release(&log_lock);
}

void klog_putsn(const char* s, uint64_t len)
{
    lock_wait(&log_lock);
    putsn(s, len);
    lock_release(&log_lock);
}

void klog_vprintf(const char* s, va_list args)
{
    lock_wait(&log_lock);
    vprintf(s, args);
    lock_release(&log_lock);
}

void klog_printf(const char* s, ...)
{
    lock_wait(&log_lock);
    va_list args;
    va_start(args, s);
    vprintf(s, args);
    va_end(args);
    lock_release(&log_lock);
}

void klog(loglevel_t lvl, const char* s, ...)
{
    lock_wait(&log_lock);
    switch (lvl) {
    case LOG_SUCCESS:
        puts("\033[32;1m[OKAY]  \033[0m");
        break;
    case LOG_WARN:
        puts("\033[33m[WARN]  \033[0m");
        break;
    case LOG_ERROR:
        puts("\033[31;1m[ERROR] \033[0m");
        break;
    default:
        puts("\033[34;1m[INFO]  \033[0m");
    }

    va_list args;
    va_start(args, s);
    vprintf(s, args);
    va_end(args);
    lock_release(&log_lock);
}

void klog_show()
{
    task_add(klogdisplayd, PRIORITY_MAX, TASK_KERNEL_MODE, NULL, 0);
}

// shows the log immediately
void klog_show_now()
{
    term_clear();
    klog_show_helper();
    term_flush();
}
