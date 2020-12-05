#include "acpi.h"
#include "kconio.h"
#include "mm/vmm.h"
#include <stddef.h>

static rsdp_t* rsdp;

void acpi_init(stv2_struct_tag_rsdp* rsdp_info)
{
    kdbg_warn("acpi_init(): STUB!\n");

    rsdp = (rsdp_t*)PHYS_TO_VIRT(rsdp_info->rsdp);

    kdbg_info("<RSDP> OEM ID: \"");
    for (size_t i = 0; i < sizeof(rsdp->OEMID); i++)
        kputchar(rsdp->OEMID[i]);

    kprintf("\". Revision: %d\n", rsdp->revision);

    if (rsdp->revision == 2) {
        kdbg_info("ACPI v2.0 found. XSDT: %x\n", rsdp->xsdt_addr);
    } else {
        kdbg_info("ACPI v1.0 found. RSDT: %x\n", rsdp->rsdt_addr);
    }
}