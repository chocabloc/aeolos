#pragma once
#include <stdint.h>

#define SYSCALL_IRQ_VECTOR 60

extern void syscall_entry(int index, uint64_t param1, uint64_t param2);
