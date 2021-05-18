#include "apic.h"
#include "../acpi/acpi.h"
#include "../acpi/madt.h"
#include "klog.h"
#include "mm/vmm.h"
#include "sys/cpu/cpu.h"
#include "sys/idt.h"
#include "timer.h"

static void* lapic_base;

__attribute__((interrupt)) static void spurious_int_handler(void* v __attribute__((unused)))
{
    klog_info("APIC spurious interrupt recieved");
}

uint32_t apic_read_reg(uint16_t offset)
{
    return *(volatile uint32_t*)(lapic_base + offset);
}

void apic_write_reg(uint16_t offset, uint32_t val)
{
    *(volatile uint32_t*)(lapic_base + offset) = val;
}

void apic_send_eoi()
{
    apic_write_reg(APIC_REG_EOI, 1);
}

// send an ipi to a processor
void apic_send_ipi(uint8_t dest, uint8_t vector, uint32_t mtype)
{
    apic_write_reg(APIC_REG_ICR_HIGH, (uint32_t)dest << 24);
    apic_write_reg(APIC_REG_ICR_LOW, (mtype << 8) | vector);
}

void apic_enable()
{
    apic_write_reg(APIC_REG_SPURIOUS_INT, APIC_FLAG_ENABLE | APIC_SPURIOUS_VECTOR_NUM);
}

void apic_init()
{
    lapic_base = (void*)PHYS_TO_VIRT(madt_get_lapic_base());
    vmm_map(NULL, (uint64_t)lapic_base, VIRT_TO_PHYS(lapic_base), 1, VMM_FLAGS_MMIO);

    // initialize the spurious interrupt register
    idt_set_handler(APIC_SPURIOUS_VECTOR_NUM, spurious_int_handler);
    apic_enable();

    // initialize the apic timer
    apic_timer_init();
    klog_ok("done\n");
}
