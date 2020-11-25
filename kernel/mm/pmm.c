/*
 *  The Physical Memory Manager
 *     (Hopefully complete)
 */

#include "pmm.h"
#include "kconio.h"
#include <stddef.h>
#include <stdint.h>

extern void kernel_start;
extern void kernel_end;

// the bitmap. 1: free, 0: used
static uint8_t* bitmap;

// size of bitmap in bytes.
static size_t bm_size;

static uint64_t total_mem;
static uint64_t free_mem;

// index of last used page
static uint64_t lastusedpage;

// flips the used/free status of a page
static void flipused(uint64_t addr)
{
    bitmap[addr / (PAGE_SIZE * PAGES_PER_BYTE)] ^= 1 << ((addr / PAGE_SIZE) % 8);
}

// check if page is free
static bool isfree(uint64_t addr)
{
    return bitmap[addr / (PAGE_SIZE * PAGES_PER_BYTE)] & (1 << ((addr / PAGE_SIZE) % 8));
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

void pmm_init(stv2_struct_tag_mmap* map)
{
    // put the bitmap at the end of the kernel
    bitmap = &kernel_end;

    kdbg_info("Memory map provided by bootloader:\n");

    // parse the memory map
    for (size_t i = 0; i < map->entries; i++) {
        struct stivale2_mmap_entry m = map->memmap[i];

        // calculate the total memory
        uint64_t new_total_mem = m.base + m.length;
        if (new_total_mem > total_mem)
            total_mem = new_total_mem;

        // if entry describes usable memory, mark it as free
        if (m.type == STIVALE2_MMAP_USABLE || m.type == STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE) {
            free_mem += m.length;
            for (size_t j = 0; j < m.length; j += PAGE_SIZE)
                flipused(m.base + j);
        }

        kprintf(" \tBase: %x, Length: %x, Type: %s\n", m.base, m.length, mmap_type_to_str(m.type));
    }

    // calculate the bitmap size
    bm_size = total_mem / (PAGE_SIZE * PAGES_PER_BYTE);

    // now mark the bitmap as used
    uint64_t bm_phys = (uint64_t)bitmap - HIGHERHALF_OFFSET;
    for (uint64_t i = bm_phys; i < bm_phys + bm_size; i += PAGE_SIZE) {
        if (isfree(i))
            flipused(i);
    }

    kdbg_info("Total mem : %d MB. Free mem: %d MB. Size of bitmap: %d bytes.\n", total_mem / (1024 * 1024), free_mem / (1024 * 1024), bm_size);
}

// get any free physical page
uint64_t pmm_get_page()
{
    for (uint64_t i = (lastusedpage * PAGE_SIZE); i < UINT64_MAX; i += PAGE_SIZE) {
        if (isfree(i)) {
            flipused(i);
            lastusedpage = i / PAGE_SIZE;
            free_mem -= PAGE_SIZE;
            return i;
        }

        // prevent overflow
        if (i + PAGE_SIZE < i)
            break;
    }

    return 0;
}

// free a page
void pmm_free_page(uint64_t page)
{
    if (!isfree(page))
        flipused(page);
}

// mark a specific page as used
// returns true if success, false otherwise
bool pmm_alloc_page(uint64_t page)
{
    if (isfree(page)) {
        flipused(page);
        return true;
    }
    return false;
}

uint64_t pmm_get_total_mem() { return total_mem; }
uint64_t pmm_get_free_mem() { return free_mem; }
