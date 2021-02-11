// The Virtual Memory Manager (some rough edges)

#include "vmm.h"
#include "dev/fb/fb.h"
#include "klog.h"
#include "memutils.h"
#include "mm/pmm.h"
#include "sys/cpu/cpu.h"
#include "sys/cpu/cpuid.h"
#include "sys/panic.h"

extern void kernel_start;
extern void kernel_end;

static uint64_t PML4[512] __attribute__((aligned(PAGE_SIZE)));

static uint64_t make_table_entry(uint64_t address, uint64_t flags)
{
    return ((address >> 12) << 12) | flags;
}

/*
 * Recursive algorithm to map a page of memory
 * Can probably be done in a better way
 */
static void _vmm_map_rec(uint64_t* table, uint64_t virtaddr, uint64_t physaddr, int plevel, uint64_t flags)
{
    // clear the pat flag if set, since it only applies to pt entries
    bool pat = false;
    uint64_t nflags = flags;
    if (nflags & FLAG_USE_PAT) {
        nflags &= ~(FLAG_USE_PAT);
        pat = true;
    }

    uint64_t shiftamt = (12 + (9 * plevel));
    uint64_t mask = 0x1FFULL << shiftamt;
    uint64_t index = (virtaddr & mask) >> shiftamt;

    if (plevel == 0) {
        // set the pat flag if pat is to be used
        nflags |= pat ? FLAG_USE_PAT : 0;
        table[index] = make_table_entry(physaddr, nflags);
        return;
    }

    uint64_t* newtable;
    if (!(table[index] & FLAG_PRESENT)) {
        newtable = (uint64_t*)PHYS_TO_VIRT(pmm_get(1));
        memset(newtable, 0, PAGE_SIZE);

        table[index] = make_table_entry(VIRT_TO_PHYS((uint64_t)newtable), nflags);
    } else
        newtable = (uint64_t*)PHYS_TO_VIRT(((table[index] >> 12) << 12));

    _vmm_map_rec(newtable, virtaddr, physaddr, plevel - 1, flags);
}

// maps virtual memory to physical (wrapper around _vmm_map_rec)
void vmm_map(uint64_t virtaddr, uint64_t physaddr, uint64_t numpages, uint64_t flags)
{
    for (uint64_t i = 0; i < numpages * PAGE_SIZE; i += PAGE_SIZE)
        _vmm_map_rec(PML4, virtaddr + i, physaddr + i, 3, flags);
}

// Create own paging structures, as the ones provided by the bootloader cannot be relied on
void vmm_init()
{
    klog_info("Identity mapping first 1MB of memory...\n");
    vmm_map(0, 0, NUM_PAGES(0x100000), FLAG_DEFAULT);

    klog_info("Mapping lower 2GB of memory to 0xFFFFFFFF80000000...\n");
    vmm_map(HIGHERHALF_OFFSET, 0, NUM_PAGES(0x80000000), FLAG_DEFAULT);

    klog_info("Mapping all memory to 0xFFFF800000000000...\n");
    vmm_map(MEM_VIRT_OFFSET, 0, NUM_PAGES(pmm_get_mem_info()->phys_limit), FLAG_DEFAULT);

    // update cr3
    write_cr("cr3", (uint64_t)&PML4 - HIGHERHALF_OFFSET);
    klog_ok("VMM initialized\n");
}
