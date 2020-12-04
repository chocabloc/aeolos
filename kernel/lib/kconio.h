#pragma once

#include <stdarg.h>

int kvprintf(const char*, va_list args);
int kprintf(const char*, ...);
int kputs(const char*);
int kputchar(int);

int kdbg_ok(const char* s, ...);
int kdbg_info(const char* s, ...);
int kdbg_warn(const char* s, ...);
int kdbg_err(const char* s, ...);
