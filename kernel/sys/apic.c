#include "apic.h"
#include "acpi/acpi.h"
#include "acpi/madt.h"
#include "kconio.h"
#include "mm/vmm.h"

static void* lapic_base;

uint32_t apic_read_reg(uint16_t offset)
{
    return *(uint32_t*)(lapic_base + offset);
}

void apic_write_reg(uint16_t offset, uint32_t val)
{
    *(uint32_t*)(lapic_base + offset) = val;
}

void apic_init()
{
    kdbg_warn("apic_init(): STUB\n");

    madt_init();
    lapic_base = (void*)PHYS_TO_VIRT(madt_get_lapic_base());
    vmm_map((uint64_t)lapic_base, VIRT_TO_PHYS(lapic_base), 1, FLAG_DEFAULT);
}