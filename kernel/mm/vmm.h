#pragma once

#include <stdint.h>

#define PML4I_MASK 0xFF8000000000
#define PDPTI_MASK 0x007FC0000000
#define PDI_MASK 0x00003FE00000
#define PTI_MASK 0x0000001FF000

#define FLAG_PRESENT 1 << 0
#define FLAG_READWRITE 1 << 1
#define FLAG_USER 1 << 2
#define FLAG_WRITETHROUGH 1 << 3
#define FLAG_CACHE_DISABLE 1 << 4

#define VIRT_TO_PHYS(a) (a - HIGHERHALF_OFFSET)
#define PHYS_TO_VIRT(a) (a + HIGHERHALF_OFFSET)

void vmm_init();