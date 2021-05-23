#pragma once

#include "boot/stivale2.h"
#include <stdbool.h>

#define PAGE_SIZE 4096
#define BMP_PAGES_PER_BYTE 8

#define HIGHERHALF_OFFSET 0xffffffff80000000

#define NUM_PAGES(num) (((num) + PAGE_SIZE - 1) / PAGE_SIZE)
#define PAGE_ALIGN_UP(num) (NUM_PAGES(num) * PAGE_SIZE)

typedef struct {
    uint64_t phys_limit;
    uint64_t total_mem;
    uint64_t free_mem;
} mem_info;

void pmm_init(stv2_struct_tag_mmap* map);
void pmm_reclaim_bootloader_mem();

uint64_t pmm_get(uint64_t numpages);
void pmm_free(uint64_t addr, uint64_t numpages);
bool pmm_alloc(uint64_t addr, uint64_t numpages);

const mem_info* pmm_getstats();
void pmm_dumpstats();
