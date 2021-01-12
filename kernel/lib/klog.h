#pragma once

#include <stdarg.h>
#include <stdint.h>

void klog_vprintf(const char* s, va_list args);
void klog_printf(const char*, ...);
void klog_puts(const char*);
void klog_putsn(const char* s, uint64_t len);
void klog_putchar(uint8_t);
void klog_puthex(uint64_t n);
void klog_putint(int n);

void klog_ok(const char* s, ...);
void klog_info(const char* s, ...);
void klog_warn(const char* s, ...);
void klog_err(const char* s, ...);

void klog_show();
void klog_hide();
