/* Really simple kmalloc and kmfree
 * works with page aligned memory
 */

#include "kmalloc.h"
#include "klog.h"
#include "memutils.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "stddef.h"

struct metadata {
    size_t numpages;
    size_t size;
};

void* kmalloc(uint64_t size)
{
    struct metadata* alloc = (struct metadata*)PHYS_TO_VIRT(pmm_get(NUM_PAGES(size) + 1));
    alloc->numpages = NUM_PAGES(size);
    alloc->size = size;
    return ((uint8_t*)alloc) + PAGE_SIZE;
}

void kmfree(void* addr)
{
    struct metadata* d = (struct metadata*)((uint8_t*)addr - PAGE_SIZE);
    pmm_free(VIRT_TO_PHYS(d), d->numpages + 1);
}

void* kmrealloc(void* addr, size_t newsize)
{
    if (!addr)
        return kmalloc(newsize);

    struct metadata* d = (struct metadata*)((uint8_t*)addr - PAGE_SIZE);
    if (NUM_PAGES(d->size) == NUM_PAGES(newsize)) {
        d->size = newsize;
        d->numpages = NUM_PAGES(newsize);
        return addr;
    }

    void* new = kmalloc(newsize);
    if (d->size > newsize)
        memcpy(addr, new, newsize);
    else
        memcpy(addr, new, d->size);

    kmfree(addr);
    return new;
}
