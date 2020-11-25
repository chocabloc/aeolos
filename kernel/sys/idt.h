#pragma once

#include <stdint.h>

#define CODE_SEGMENT_SELECTOR 0x08

#define IDT_FLAGS_DEFAULT 0b1000111000000000

struct idt_entry {
    uint16_t offset_15_0;
    uint16_t selector;
    uint16_t flags;
    uint16_t offset_31_16;
    uint32_t offset_63_32;
    uint32_t reserved;
} __attribute__((packed));

struct idtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_init();