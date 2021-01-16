#include "smp.h"
#include "acpi/madt.h"
#include "klog.h"
#include "memutils.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "sys/apic/apic.h"
#include "sys/cpu/cpu.h"

// smp trampoline code to be executed by AP's
extern void smp_trampoline_blob_start, smp_trampoline_blob_end;

// counter to be incremented upon successful AP boot
static volatile int* ap_boot_counter = (volatile int*)PHYS_TO_VIRT(SMP_AP_BOOT_COUNTER_ADDR);

void smp_init()
{
    klog_warn("smp_init(): STUB\n");

    // copy the trampoline blob to 0x1000 physical
    uint64_t trmpblobsize = (uint64_t)&smp_trampoline_blob_end - (uint64_t)&smp_trampoline_blob_start;
    klog_info("SMP Trampoline code size: %d bytes\n", trmpblobsize);
    memcpy(&smp_trampoline_blob_start, (void*)PHYS_TO_VIRT(SMP_TRAMPOLINE_BLOB_ADDR), trmpblobsize);

    // pass arguments to trampoline code
    read_cr("cr3", (uint64_t*)PHYS_TO_VIRT(SMP_TRAMPOLINE_ARG_CR3));
    asm volatile("sidt %0"
                 : "=m"(*(uint64_t*)PHYS_TO_VIRT(SMP_TRAMPOLINE_ARG_IDTPTR))
                 :
                 :);

    // get lapic info from the madt
    uint64_t cpunum = madt_get_num_lapic();
    madt_record_lapic** lapics = madt_get_lapics();
    klog_info("Number of CPU's: %d\n", cpunum);

    // loop through the lapic's present and initialize them one by one
    for (uint64_t i = 0; i < cpunum; i++) {
        // if cpu is the bootstrap processor, do not initialize it
        if (apic_read_reg(APIC_REG_ID) == lapics[i]->apic_id) {
            klog_info("CPU %d is BSP\n", lapics[i]->apic_id);
            continue;
        }
        klog_info("Initializing CPU %d...", lapics[i]->apic_id);

        // send the init ipi
        apic_send_ipi(lapics[i]->apic_id, 0, APIC_IPI_TYPE_INIT);

        // send the startup ipi
        apic_send_ipi(lapics[i]->apic_id, SMP_TRAMPOLINE_BLOB_ADDR / PAGE_SIZE, APIC_IPI_TYPE_STARTUP);

        // check if cpu has started
        bool success = false;
        int counter_prev = *ap_boot_counter, counter_curr = counter_prev;
        for (int j = 0; j < 20000000; j++) {
            counter_curr = *ap_boot_counter;

            if (counter_curr != counter_prev) {
                success = true;
                break;
            }
        }

        if (!success)
            klog_printf(" Failed\n");
        else
            klog_printf(" Done\n");
    }
}