/* Really simple kmalloc and kmfree
 * works with page aligned memory
 */

#include "kmalloc.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "stddef.h"

void* kmalloc(uint64_t size)
{
    return (void*)PHYS_TO_VIRT(pmm_get(NUM_PAGES(size)));
}

void kmfree(void* addr, uint64_t size)
{
    pmm_free(VIRT_TO_PHYS((uint64_t)addr), NUM_PAGES(size));
}