#include "gdt.h"
#include "kmalloc.h"
#include <stdint.h>

void gdt_init()
{
    gdt_t* gdt = (gdt_t*)kmalloc(sizeof(gdt_t));
    *gdt = (gdt_t) {
        .entry_null = GDT_ENTRY_NULL,
        .entry_kcode = GDT_ENTRY_KERNEL_CODE,
        .entry_kdata = GDT_ENTRY_KERNEL_DATA,
        .entry_ucode = GDT_ENTRY_USER_CODE,
        .entry_udata = GDT_ENTRY_USER_DATA
    };

    struct gdtr g = { .base = (uint64_t)gdt, .limit = sizeof(gdt_t) - 1};
    asm volatile("lgdt %0;"
                 "pushq $0x08;"
                 "pushq $reload_sr;"
                 "lretq;"
                 "reload_sr:"
                 "movw $0x10, %%ax;"
                 "movw %%ax, %%ds;"
                 "movw %%ax, %%es;"
                 "movw %%ax, %%ss;"
                 "movw %%ax, %%fs;"
                 "movw %%ax, %%gs;"
                 :
                 : "g"(g)
                 :);
}

void gdt_install_tss(tss_t* tss)
{
    struct gdtr gdtr;
    asm volatile("sgdt %0"
                 :
                 : "m"(gdtr)
                 : "memory");

    gdt_t* gdt = (gdt_t*)(gdtr.base);
    uint64_t baseaddr = (uint64_t)tss;
    uint64_t seglimit = sizeof(tss_t);

    gdt->tss.base_addr_0_15 = baseaddr & 0xFFFF;
    gdt->tss.base_addr_16_23 = (baseaddr >> 16) & 0xFF;
    gdt->tss.base_addr_24_31 = (baseaddr >> 24) & 0xFF;
    gdt->tss.base_addr_32_63 = baseaddr >> 32;
    gdt->tss.seg_limit_0_15 = seglimit & 0xFFFF;
    gdt->tss.flags_low = 0x89;
    gdt->tss.flags_high = 0;

    asm volatile("mov $0x28, %%ax;"
                 "ltr %%ax"
                 :
                 :
                 : "ax");
}
