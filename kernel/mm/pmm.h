#pragma once

#include "boot/stivale2.h"
#include <stdbool.h>

#define PAGE_SIZE 4096
#define PAGES_PER_BYTE 8

#define HIGHERHALF_OFFSET 0xffffffff80000000

void pmm_init(stv2_struct_tag_mmap* map);

uint64_t pmm_get_page();
void pmm_free_page(uint64_t);
bool pmm_alloc_page(uint64_t);

uint64_t pmm_get_total_mem();
uint64_t pmm_get_free_mem();
uint64_t pmm_get_bm_end();
