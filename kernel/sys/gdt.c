#include "gdt.h"
#include <stdint.h>

static const uint64_t GDT[3] = { GDT_ENTRY_NULL, GDT_ENTRY_CODE, GDT_ENTRY_DATA };

extern void gdt_load(struct gdtr*);

void gdt_init()
{
    struct gdtr g = { .base = (uint64_t)&GDT,
        .limit = (3 * 8 - 1) };

    gdt_load(&g);
}