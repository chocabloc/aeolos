// the PMM

#include "pmm.h"
#include "dev/fb/fb.h"
#include "klog.h"
#include "memutils.h"
#include "sys/panic.h"
#include "vmm.h"
#include <stddef.h>
#include <stdint.h>

extern void kernel_end;

// the bitmap
static uint8_t* bitmap;

// the memory map
static stv2_struct_tag_mmap* mmap;

// memory stats: total mem, free mem, etc
static mem_info memstats;

static void bmp_markused(uint64_t addr, uint64_t numpages)
{
    for (uint64_t i = addr; i < addr + (numpages * PAGE_SIZE); i += PAGE_SIZE) {
        bitmap[i / (PAGE_SIZE * BMP_PAGES_PER_BYTE)] &= ~((1 << ((i / PAGE_SIZE) % BMP_PAGES_PER_BYTE)));
    }
}

static bool bmp_isfree(uint64_t addr, uint64_t numpages)
{
    bool free = true;

    for (uint64_t i = addr; i < addr + (numpages * PAGE_SIZE); i += PAGE_SIZE) {
        free = bitmap[i / (PAGE_SIZE * BMP_PAGES_PER_BYTE)] & (1 << ((i / PAGE_SIZE) % BMP_PAGES_PER_BYTE));
        if (!free)
            break;
    }
    return free;
}

// marks pages as free
void pmm_free(uint64_t addr, uint64_t numpages)
{
    for (uint64_t i = addr; i < addr + (numpages * PAGE_SIZE); i += PAGE_SIZE) {
        if (!bmp_isfree(i, 1))
            memstats.free_mem += PAGE_SIZE;

        bitmap[i / (PAGE_SIZE * BMP_PAGES_PER_BYTE)] |= 1 << ((i / PAGE_SIZE) % BMP_PAGES_PER_BYTE);
    }
}

// marks pages as used, returns true if success, false otherwise
bool pmm_alloc(uint64_t addr, uint64_t numpages)
{
    if (!bmp_isfree(addr, numpages))
        return false;

    bmp_markused(addr, numpages);
    memstats.free_mem -= numpages * PAGE_SIZE;
    return true;
}

uint64_t pmm_get(uint64_t numpages)
{
    static uint64_t lastusedpage;

    for (uint64_t i = lastusedpage; i < memstats.phys_limit; i += PAGE_SIZE) {
        if (pmm_alloc(i, numpages))
            return i;
    }

    for (uint64_t i = 0; i < lastusedpage; i += PAGE_SIZE) {
        if (pmm_alloc(i, numpages))
            return i;
    }

    kernel_panic("Out of Physical Memory");
    return 0;
}

void pmm_init(stv2_struct_tag_mmap* map)
{
    mmap = map;

    // calculate memory statistics
    for (size_t i = 0; i < map->entries; i++) {
        struct stivale2_mmap_entry entry = map->memmap[i];

        if (entry.base + entry.length <= 0x100000)
            continue;

        uint64_t newphyslimit = entry.base + entry.length;
        if (newphyslimit > memstats.phys_limit)
            memstats.phys_limit = newphyslimit;

        if (entry.type == STIVALE2_MMAP_USABLE || entry.type == STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE
            || entry.type == STIVALE2_MMAP_ACPI_RECLAIMABLE || entry.type == STIVALE2_MMAP_KERNEL_AND_MODULES)
            memstats.total_mem += entry.length;
    }
    klog_info("physical limit = %x. total memory = %d MB\n", memstats.phys_limit, memstats.total_mem / (1024 * 1024));

    // look for a good place to keep our bitmap
    uint64_t bm_size = memstats.phys_limit / (PAGE_SIZE * BMP_PAGES_PER_BYTE);
    for (size_t i = 0; i < map->entries; i++) {
        struct stivale2_mmap_entry entry = map->memmap[i];

        if (entry.base + entry.length <= 0x100000)
            continue;

        if (entry.length >= bm_size && entry.type == STIVALE2_MMAP_USABLE) {
            bitmap = (uint8_t*)PHYS_TO_VIRT(entry.base);
            break;
        }
    }
    // zero it out
    memset(bitmap, 0, bm_size);

    // now populate the bitmap
    for (size_t i = 0; i < map->entries; i++) {
        struct stivale2_mmap_entry entry = map->memmap[i];

        if (entry.base + entry.length <= 0x100000)
            continue;

        if (entry.type == STIVALE2_MMAP_USABLE)
            pmm_free(entry.base, NUM_PAGES(entry.length));
    }

    // mark the bitmap as used
    pmm_alloc(VIRT_TO_PHYS(bitmap), NUM_PAGES(bm_size));

    klog_ok("done\n");
}

// reclaim memory used by bootloader
void pmm_reclaim_bootloader_mem()
{
    for (size_t i = 0; i < mmap->entries; i++) {
        struct stivale2_mmap_entry entry = mmap->memmap[i];

        if (entry.type == STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE)
            pmm_free(entry.base, NUM_PAGES(entry.length));
    }
}

const mem_info* pmm_get_mem_info() { return &memstats; }
