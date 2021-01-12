#include "gdt.h"
#include "klog.h"
#include <stdint.h>

static const uint64_t GDT[3] = { GDT_ENTRY_NULL, GDT_ENTRY_CODE, GDT_ENTRY_DATA };

extern void gdt_load(struct gdtr*);

void gdt_init()
{
    struct gdtr g = { .base = (uint64_t)&GDT,
        .limit = (3 * 8 - 1) };

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

    klog_ok("GDT initialized\n");
}