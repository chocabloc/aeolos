#include "acpi.h"
#include "kconio.h"
#include "madt.h"
#include "memutils.h"
#include "mm/vmm.h"
#include <stdbool.h>
#include <stddef.h>

static acpi_sdt* rsdt;
static acpi_sdt* xsdt;

static bool xsdt_present;

acpi_sdt* acpi_get_sdt(const char* sign)
{
    if (xsdt_present) {
        uint64_t len = (xsdt->hdr.length - sizeof(acpi_sdt)) / sizeof(uint64_t);
        for (uint64_t i = 0; i < len; i++) {
            acpi_sdt* table = (acpi_sdt*)PHYS_TO_VIRT(((uint64_t*)xsdt->data)[i]);
            if (memcmp(table->hdr.sign, sign, sizeof(table->hdr.sign))) {
                kdbg_info("Found ACPI SDT \"%s\"\n", sign);
                return table;
            }
        }
    } else {
        uint64_t len = (rsdt->hdr.length - sizeof(acpi_sdt)) / sizeof(uint32_t);
        for (uint64_t i = 0; i < len; i++) {
            acpi_sdt* table = (acpi_sdt*)PHYS_TO_VIRT(((uint32_t*)rsdt->data)[i]);
            if (memcmp(table->hdr.sign, sign, sizeof(table->hdr.sign))) {
                kdbg_info("Found ACPI SDT \"%s\"\n", sign);
                return table;
            }
        }
    }

    kdbg_warn("ACPI SDT \"%s\" not found.\n", sign);
    return NULL;
}

void acpi_init(stv2_struct_tag_rsdp* rsdp_info)
{
    rsdp_t* rsdp = (rsdp_t*)PHYS_TO_VIRT(rsdp_info->rsdp);

    kdbg_info("RSDP OEM ID: \"");
    kputsn(rsdp->oem_id, sizeof(rsdp->oem_id));
    kprintf("\". Revision: %d\n", rsdp->revision);

    if (rsdp->revision == 2) {
        kdbg_info("ACPI v2.0 detected\n");
        xsdt = (acpi_sdt*)PHYS_TO_VIRT(rsdp->xsdt_addr);
        xsdt_present = true;
    } else {
        kdbg_info("ACPI v1.0 detected\n");
        rsdt = (acpi_sdt*)PHYS_TO_VIRT(rsdp->rsdt_addr);
        xsdt_present = false;
    }

    kdbg_ok("ACPI tables initialized\n");
}
