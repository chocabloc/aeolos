#pragma once

#include "lock.h"
#include <stdint.h>

#define MEM_VIRT_OFFSET 0xffff800000000000

#define VMM_FLAG_PRESENT 1 << 0
#define VMM_FLAG_READWRITE 1 << 1
#define VMM_FLAG_USER 1 << 2
#define VMM_FLAG_WRITETHROUGH 1 << 3
#define VMM_FLAG_CACHE_DISABLE 1 << 4
#define VMM_FLAG_WRITECOMBINE 1 << 7

#define VMM_FLAGS_DEFAULT (VMM_FLAG_PRESENT | VMM_FLAG_READWRITE)
#define VMM_FLAGS_MMIO (VMM_FLAGS_DEFAULT | VMM_FLAG_CACHE_DISABLE)
#define VMM_FLAGS_USERMODE (VMM_FLAGS_DEFAULT | VMM_FLAG_USER)

#define VIRT_TO_PHYS(a) (((uint64_t)(a)) - MEM_VIRT_OFFSET)
#define PHYS_TO_VIRT(a) (((uint64_t)(a)) + MEM_VIRT_OFFSET)

typedef struct {
    uint64_t* PML4;
    lock_t lock;
} addrspace_t;

void vmm_init();
void vmm_map(addrspace_t* addrspace, uint64_t vaddr, uint64_t paddr, uint64_t np, uint64_t flags);
void vmm_unmap(addrspace_t* addrspace, uint64_t vaddr, uint64_t np);
