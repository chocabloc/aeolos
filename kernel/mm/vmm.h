#pragma once

#include <stdint.h>

#define MEM_VIRT_OFFSET 0xffff800000000000

#define FLAG_PRESENT 1 << 0
#define FLAG_READWRITE 1 << 1
#define FLAG_USER 1 << 2
#define FLAG_WRITETHROUGH 1 << 3
#define FLAG_CACHE_DISABLE 1 << 4

#define VIRT_TO_PHYS(a) ((uint64_t)(a)-MEM_VIRT_OFFSET)
#define PHYS_TO_VIRT(a) ((uint64_t)(a) + MEM_VIRT_OFFSET)

void vmm_init();