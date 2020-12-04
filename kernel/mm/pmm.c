/*
 *  The Physical Memory Manager (Hopefully complete)
 *   if you copy any part of this code, make sure to attribute Aditya Arsh
 */

#include "pmm.h"
#include "dev/fb/fb.h"
#include "kconio.h"
#include "sys/panic.h"
#include <stddef.h>
#include <stdint.h>

extern void kernel_end;

// the bitmap. 1: free, 0: used
static uint8_t* bitmap;

// size of bitmap in bytes.
static size_t bm_size;

// memory stats: total mem, free mem, etc
static mem_info memstats;

// marks pages as used
static void bmp_markused(uint64_t addr, uint64_t numpages)
{
    for (uint64_t i = addr; i < addr + (numpages * PAGE_SIZE); i += PAGE_SIZE) {
        bitmap[i / (PAGE_SIZE * BMP_PAGES_PER_BYTE)] &= ~(1 << ((i / PAGE_SIZE) % 8));
    }
}

// check if pages are free
static bool bmp_isfree(uint64_t addr, uint64_t numpages)
{
    uint64_t end = addr + (numpages * PAGE_SIZE);
    if (end >= memstats.phys_limit)
        return false;

    bool free = true;
    for (uint64_t i = addr; i < end; i += PAGE_SIZE) {
        free = bitmap[i / (PAGE_SIZE * BMP_PAGES_PER_BYTE)] & (1 << ((i / PAGE_SIZE) % 8));
        if (!free)
            break;
    }
    return free;
}

// convert a memory map type to human readable string
static const char* mmap_type_to_str(uint64_t type)
{
    switch (type) {
    case STIVALE2_MMAP_USABLE:
        return "Usable";
        break;
    case STIVALE2_MMAP_RESERVED:
        return "Reserved";
        break;
    case STIVALE2_MMAP_ACPI_RECLAIMABLE:
        return "ACPI (Reclaimable)";
        break;
    case STIVALE2_MMAP_ACPI_NVS:
        return "ACPI Non-Volatile Storage";
        break;
    case STIVALE2_MMAP_BAD_MEMORY:
        return "Faulty Memory";
        break;
    case STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE:
        return "Bootloader (Reclaimable)";
        break;
    case STIVALE2_MMAP_KERNEL_AND_MODULES:
        return "Kernel and/or Modules";
        break;
    default:
        return "Invalid Memory Type";
        break;
    }
}

// marks pages as free
void pmm_free(uint64_t addr, uint64_t numpages)
{
    for (uint64_t i = addr; i < addr + (numpages * PAGE_SIZE); i += PAGE_SIZE) {
        bitmap[i / (PAGE_SIZE * BMP_PAGES_PER_BYTE)] |= 1 << ((i / PAGE_SIZE) % 8);
    }
}

/* marks pages as used
 * returns true if success, false otherwise
 */
bool pmm_alloc(uint64_t addr, uint64_t numpages)
{
    if (bmp_isfree(addr, numpages)) {
        bmp_markused(addr, numpages);
        memstats.free_mem -= numpages * PAGE_SIZE;
        return true;
    } else {
        return false;
    }
}

uint64_t pmm_get(uint64_t numpages)
{
    static uint64_t lastusedpage;

    // first search after last used page
    for (uint64_t i = lastusedpage; i < memstats.phys_limit; i += numpages * PAGE_SIZE) {
        if (pmm_alloc(i, numpages))
            return i;
    }

    // if not found, search from start to last used page
    for (uint64_t i = 0; i < lastusedpage; i += numpages * PAGE_SIZE) {
        if (pmm_alloc(i, numpages))
            return i;
    }

    kernel_panic("Out Of Physical Memory");
    return 0;
}

void pmm_init(stv2_struct_tag_mmap* map)
{
    // put the bitmap at the end of the kernel
    bitmap = &kernel_end;

    kdbg_info("Memory map provided by bootloader:\n");

    // parse the memory map
    for (size_t i = 0; i < map->entries; i++) {
        struct stivale2_mmap_entry m = map->memmap[i];

        // calculate the physical limit
        uint64_t new_phys_limit = m.base + m.length;
        if (new_phys_limit > memstats.phys_limit)
            memstats.phys_limit = new_phys_limit;

        // if entry describes usable memory, mark it as free
        if (m.type == STIVALE2_MMAP_USABLE || m.type == STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE) {
            memstats.total_mem += m.length;
            pmm_free(m.base, NUM_PAGES(m.length));
        }

        kprintf(" \tBase: %x, Length: %x, Type: %s\n", m.base, m.length, mmap_type_to_str(m.type));
    }
    memstats.free_mem = memstats.total_mem;

    // mark first 1MB as used
    pmm_alloc(0, 256);

    // calculate the bitmap size
    bm_size = memstats.phys_limit / (PAGE_SIZE * BMP_PAGES_PER_BYTE);

    // now mark the bitmap as used
    uint64_t bm_phys = (uint64_t)bitmap - HIGHERHALF_OFFSET;
    pmm_alloc(bm_phys, NUM_PAGES(bm_size));

    kdbg_info("Physical mem limit: %x. Total mem: %d MB. Free mem: %d MB. Size of bitmap: %d bytes.\n",
        memstats.phys_limit, memstats.total_mem / (1024 * 1024), memstats.free_mem / (1024 * 1024), bm_size);
}

const mem_info* pmm_get_mem_info() { return &memstats; }
