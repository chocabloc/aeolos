#include "acpi/acpi.h"
#include "acpi/madt.h"
#include "kconio.h"
#include "mm/vmm.h"

static void* lapic_base;

uint32_t apic_read_reg(uint16_t offset)
{
    return 0;
}

void apic_init()
{
    kdbg_warn("apic_init(): STUB\n");

    madt_init();
    lapic_base = (void*)PHYS_TO_VIRT(madt_get_lapic_base());
}