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

static uint64_t make_table_entry(uint64_t address, uint64_t flags)
{
    return ((address >> 12) << 12) | flags;
}

void vmm_init()
{
    kdbg_info("vmm_init(): STUB!!\n");
    kdbg_info("PML4 is at %x.\n", &PML4);
    kdbg_info("Index in PML4 is %d\n", get_pml4_offset(HIGHERHALF_OFFSET));
    kdbg_info("Mapping kernel...\n");

    // do fractal mapping
    PML4[510] = make_table_entry((uint64_t)&PML4 - HIGHERHALF_OFFSET, FLAG_PRESENT | FLAG_READWRITE);

    // now map the kernel, starting at the higher half offset to the kernel end
    uint64_t kernel_pml4i = get_pml4_offset(HIGHERHALF_OFFSET);
    for (uint64_t a = HIGHERHALF_OFFSET; a <= (uint64_t)&kernel_end; a += PAGE_SIZE) {
        uint64_t pml4i = get_pml4_offset(a);
        if (!(PML4[pml4i] & FLAG_PRESENT)) {
            PML4[pml4i] = make_table_entry(pmm_get_page(), FLAG_PRESENT | FLAG_READWRITE);
            kprintf("%x\n", PML4[pml4i]);
        }
    }

    kdbg_info("Updating CR3...\n");
    // update cr3
    asm("movq %0, %%rax; movq %%rax, %%cr3;"
        :
        : "g"((uint64_t)&PML4 - HIGHERHALF_OFFSET)
        : "rax");
}