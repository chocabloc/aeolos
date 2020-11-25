#pragma once

int kprintf(const char*, ...);
int kputs(const char*);
int kputchar(int);

int kdbg_ok(const char* s, ...);
int kdbg_info(const char* s, ...);