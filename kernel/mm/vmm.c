/* The Virtual Memory Manager
 *   (some rough edges)
 */

#include "vmm.h"
#include "dev/fb/fb.h"
#include "dev/term/term.h"
#include "kconio.h"
#include "mm/pmm.h"

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
static void _vmm_map_rec(uint64_t* table, uint64_t virtaddr, uint64_t physaddr, int plevel)
{
    uint64_t shiftamt = (12 + (9 * plevel));
    uint64_t mask = 0x1FFULL << shiftamt;
    uint64_t index = (virtaddr & mask) >> shiftamt;

    if (plevel == 0) {
        table[index] = make_table_entry(physaddr, FLAG_PRESENT | FLAG_READWRITE);
        return;
    }

    uint64_t* newtable;
    if (!(table[index] & FLAG_PRESENT)) {
        newtable = (uint64_t*)PHYS_TO_VIRT(pmm_get_page());

        for (int i = 0; i < 512; i++)
            newtable[i] = 0;

        table[index] = make_table_entry(VIRT_TO_PHYS((uint64_t)newtable), FLAG_PRESENT | FLAG_READWRITE);
    } else
        newtable = (uint64_t*)PHYS_TO_VIRT(((table[index] >> 12) << 12));

    _vmm_map_rec(newtable, virtaddr, physaddr, plevel - 1);
}

/*
 * map virtual memory to physical
 * just a wrapper around _vmm_map_rec
 */
void vmm_map(uint64_t virtaddr, uint64_t physaddr, uint64_t numpages)
{
    for (uint64_t i = 0; i < numpages * PAGE_SIZE; i += PAGE_SIZE)
        _vmm_map_rec(PML4, virtaddr + i, physaddr + i, 3);
}

/*
 * Create own paging structures, as the ones provided by the bootloader cannot be relied on
 * Map the kernel, the PMM data structures, and the framebuffer
 */
void vmm_init()
{
    kdbg_info("Mapping kernel...\n");
    vmm_map(HIGHERHALF_OFFSET, 0, (VIRT_TO_PHYS(pmm_get_bm_end()) / PAGE_SIZE) + 1);

    kdbg_info("Remapping framebuffer...\n");
    const fb_info* oldfb = fb_getinfo();

    // put the new framebuffer just after the pmm data structures, page aligned
    uint64_t newfbaddr = ((pmm_get_bm_end() + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    uint64_t fbsize = (oldfb->pitch * oldfb->height) / PAGE_SIZE;

    vmm_map(newfbaddr, (uint64_t)oldfb->addr, fbsize);
    fb_remap(newfbaddr);

    // update cr3
    asm("movq %0, %%rax; movq %%rax, %%cr3;"
        :
        : "g"(VIRT_TO_PHYS((uint64_t)&PML4))
        : "rax");

    kdbg_info("Updated CR3\n");
}