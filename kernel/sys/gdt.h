#pragma once

#include "sys/smp/smp.h"
#include <stdint.h>

#define GDT_ENTRY_NULL 0x0000000000000000
#define GDT_ENTRY_KERNEL_CODE 0x00AF9A000000FFFF
#define GDT_ENTRY_KERNEL_DATA 0x008F92000000FFFF
#define GDT_ENTRY_USER_CODE 0x00AFFA000000FFFF
#define GDT_ENTRY_USER_DATA 0x008FF2000000FFFF

typedef struct {
    uint16_t seg_limit_0_15;
    uint16_t base_addr_0_15;
    uint8_t base_addr_16_23;
    uint8_t flags_low;
    uint8_t flags_high;
    uint8_t base_addr_24_31;
    uint32_t base_addr_32_63;
    uint32_t reserved;
} __attribute__((packed)) sys_seg_desc_t;

typedef struct {
    uint64_t entry_null;
    uint64_t entry_kcode;
    uint64_t entry_kdata;
    uint64_t entry_ucode;
    uint64_t entry_udata;
    sys_seg_desc_t tss;
} __attribute__((packed)) gdt_t;

struct gdtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void gdt_init();
void gdt_install_tss(tss_t* tss);
