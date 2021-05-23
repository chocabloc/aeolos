#include "vmm.h"
#include "klog.h"
#include "kmalloc.h"
#include "memutils.h"
#include "mm/pmm.h"
#include "sys/cpu/cpu.h"

#define MAKE_TABLE_ENTRY(address, flags) ((address & ~(0xfff)) | flags)

static addrspace_t kaddrspace;

static void map_page(addrspace_t* addrspace, uint64_t vaddr, uint64_t paddr, uint64_t flags)
{
    uint16_t pte = (vaddr >> 12) & 0x1ff;
    uint16_t pde = (vaddr >> 21) & 0x1ff;
    uint16_t pdpe = (vaddr >> 30) & 0x1ff;
    uint16_t pml4e = (vaddr >> 39) & 0x1ff;

    uint64_t* pml4 = addrspace->PML4;
    uint64_t* pdpt;
    uint64_t* pd;
    uint64_t* pt;

    pdpt = (uint64_t*)PHYS_TO_VIRT(pml4[pml4e] & ~(0xfff));
    if (!(pml4[pml4e] & VMM_FLAG_PRESENT)) {
        pdpt = (uint64_t*)PHYS_TO_VIRT(pmm_get(1));
        memset(pdpt, 0, PAGE_SIZE);
        pml4[pml4e] = MAKE_TABLE_ENTRY(VIRT_TO_PHYS(pdpt), VMM_FLAGS_USERMODE);
    }

    pd = (uint64_t*)PHYS_TO_VIRT(pdpt[pdpe] & ~(0xfff));
    if (!(pdpt[pdpe] & VMM_FLAG_PRESENT)) {
        pd = (uint64_t*)PHYS_TO_VIRT(pmm_get(1));
        memset(pd, 0, PAGE_SIZE);
        pdpt[pdpe] = MAKE_TABLE_ENTRY(VIRT_TO_PHYS(pd), VMM_FLAGS_USERMODE);
    }

    pt = (uint64_t*)PHYS_TO_VIRT(pd[pde] & ~(0xfff));
    if (!(pd[pde] & VMM_FLAG_PRESENT)) {
        pt = (uint64_t*)PHYS_TO_VIRT(pmm_get(1));
        memset(pt, 0, PAGE_SIZE);
        pd[pde] = MAKE_TABLE_ENTRY(VIRT_TO_PHYS(pt), VMM_FLAGS_USERMODE);
    }

    pt[pte] = MAKE_TABLE_ENTRY(paddr & ~(0xfff), flags);

    uint64_t cr3val;
    read_cr("cr3", &cr3val);
    if (cr3val == (uint64_t)(VIRT_TO_PHYS(addrspace->PML4)))
        asm volatile("invlpg (%0)" ::"r"(vaddr));
}

static void unmap_page(addrspace_t* addrspace, uint64_t vaddr)
{
    uint16_t pte = (vaddr >> 12) & 0x1ff;
    uint16_t pde = (vaddr >> 21) & 0x1ff;
    uint16_t pdpe = (vaddr >> 30) & 0x1ff;
    uint16_t pml4e = (vaddr >> 39) & 0x1ff;

    uint64_t* pml4 = addrspace->PML4;
    if (!(pml4[pml4e] & VMM_FLAG_PRESENT))
        return;

    uint64_t* pdpt = (uint64_t*)PHYS_TO_VIRT(pml4[pml4e] & ~(0x1ff));
    if (!(pdpt[pdpe] & VMM_FLAG_PRESENT))
        return;

    uint64_t* pd = (uint64_t*)PHYS_TO_VIRT(pdpt[pdpe] & ~(0x1ff));
    if (!(pd[pde] & VMM_FLAG_PRESENT))
        return;

    uint64_t* pt = (uint64_t*)PHYS_TO_VIRT(pd[pde] & ~(0x1ff));
    if (!(pt[pte] & VMM_FLAG_PRESENT))
        return;

    pt[pte] = 0;

    uint64_t cr3val;
    read_cr("cr3", &cr3val);
    if (cr3val == (uint64_t)(VIRT_TO_PHYS(addrspace->PML4)))
        asm volatile("invlpg (%0)" ::"r"(vaddr));

    for (int i = 0; i < 512; i++)
        if (pt[i] != 0)
            goto done;
    pd[pde] = 0;
    pmm_free(VIRT_TO_PHYS(pt), 1);

    for (int i = 0; i < 512; i++)
        if (pd[i] != 0)
            goto done;
    pdpt[pdpe] = 0;
    pmm_free(VIRT_TO_PHYS(pd), 1);

    for (int i = 0; i < 512; i++)
        if (pdpt[i] != 0)
            goto done;
    pml4[pml4e] = 0;
    pmm_free(VIRT_TO_PHYS(pdpt), 1);

done:
    return;
}

void vmm_unmap(addrspace_t* addrspace, uint64_t vaddr, uint64_t np)
{
    addrspace_t* as = addrspace ? addrspace : &kaddrspace;
    for (size_t i = 0; i < np * PAGE_SIZE; i += PAGE_SIZE)
        unmap_page(as, vaddr + i);
}

void vmm_map(addrspace_t* addrspace, uint64_t vaddr, uint64_t paddr, uint64_t np, uint64_t flags)
{
    addrspace_t* as = addrspace ? addrspace : &kaddrspace;
    for (size_t i = 0; i < np * PAGE_SIZE; i += PAGE_SIZE)
        map_page(as, vaddr + i, paddr + i, flags);
}

// Create own paging structures, as the ones provided by the bootloader cannot be relied on
void vmm_init()
{
    // create the kernel address space
    kaddrspace.PML4 = kmalloc(PAGE_SIZE);
    memset(kaddrspace.PML4, 0, PAGE_SIZE);

    vmm_map(&kaddrspace, 0xffffffff80000000, 0, NUM_PAGES(0x80000000), VMM_FLAGS_DEFAULT);
    klog_info("mapped lower 2GB to 0xFFFFFFFF80000000\n");

    vmm_map(&kaddrspace, 0xffff800000000000, 0, NUM_PAGES(pmm_getstats()->phys_limit), VMM_FLAGS_DEFAULT);
    klog_info("mapped all memory to 0xFFFF800000000000\n");

    write_cr("cr3", VIRT_TO_PHYS(kaddrspace.PML4));
    klog_ok("done\n");
}
