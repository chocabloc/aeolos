#include "vmm.h"
#include "kconio.h"
#include "mm/pmm.h"

extern void kernel_start;
extern void kernel_end;

static uint64_t PML4[512] __attribute__((aligned(PAGE_SIZE)));

static uint64_t get_pml4_offset(uint64_t addr)
{
    return (addr & PML4E_MASK) >> ((9 * 3) + 12);
}

void vmm_init()
{
    kdbg_info("vmm_init(): STUB!!\n");
    kdbg_info("PML4 is at %x.\n", &PML4);
    kdbg_info("Index in PML4 is %d\n", get_pml4_offset(HIGHERHALF_OFFSET));
}