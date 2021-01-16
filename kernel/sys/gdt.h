#pragma once

#include <stdint.h>

#define GDT_ENTRY_NULL 0x0000000000000000
#define GDT_ENTRY_CODE 0x00AF9A000000FFFF
#define GDT_ENTRY_DATA 0x008F92000000FFFF

struct gdtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void gdt_init();