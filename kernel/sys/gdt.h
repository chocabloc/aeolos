#pragma once

#include <stdint.h>

#define GDT_ENTRY_NULL 0x0000000000000000
#define GDT_ENTRY_CODE 0x00209A0000000000
#define GDT_ENTRY_DATA 0x0000920000000000

struct gdtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void gdt_init();