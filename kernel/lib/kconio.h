#pragma once

#include <stdarg.h>
#include <stdint.h>

int kvprintf(const char*, va_list args);
int kprintf(const char*, ...);
int kputs(const char*);
int kputsn(const char* s, uint64_t len);
int kputchar(int);

int kdbg_ok(const char* s, ...);
int kdbg_info(const char* s, ...);
int kdbg_warn(const char* s, ...);
int kdbg_err(const char* s, ...);
