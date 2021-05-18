#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define KLOG_BUFF_LEN (UINT16_MAX + 1)

typedef enum {
    LOG_SUCCESS,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} loglevel_t;

void klog_putchar(uint8_t);
void klog_puts(const char* s);
void klog_printf(const char*, ...);
void klog_vprintf(const char*, va_list);
void klog(loglevel_t lvl, const char* s, ...);
void klog_show();
void klog_show_urgent();

#define klog_ok(s, ...) klog(LOG_SUCCESS, "%s(): " s, __func__, ##__VA_ARGS__)
#define klog_info(s, ...) klog(LOG_INFO, "%s(): " s, __func__, ##__VA_ARGS__)
#define klog_warn(s, ...) klog(LOG_WARN, "%s(): " s, __func__, ##__VA_ARGS__)
#define klog_err(s, ...) klog(LOG_ERROR, "%s(): " s, __func__, ##__VA_ARGS__)
