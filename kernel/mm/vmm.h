#pragma once

#include <stdint.h>

#define PML4E_MASK 0xFF8000000000

#define FLAG_PRESENT 1 << 0
#define FLAG_READWRITE 1 << 1
#define FLAG_USER 1 << 2
#define FLAG_WRITETHROUGH 1 << 3
#define FLAG_CACHE_DISABLE 1 << 4

void vmm_init();