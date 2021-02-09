#include "smp.h"
#include "../acpi/madt.h"
#include "klog.h"
#include "memutils.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "sys/apic/apic.h"
#include "sys/cpu/cpu.h"
#include "sys/pit.h"

// smp trampoline code to be executed by AP's
extern void smp_trampoline_blob_start, smp_trampoline_blob_end;

// counter to be incremented upon successful AP boot
static volatile int* ap_boot_counter = (volatile int*)PHYS_TO_VIRT(SMP_AP_BOOT_COUNTER_ADDR);

static smp_info_t info;

const smp_info_t* smp_get_info()
{
    return &info;
}

void smp_init()
{
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
        int counter_prev = *ap_boot_counter, counter_curr = counter_prev;

        // if cpu is not online capable, do not initialize it
        if (!(lapics[i]->flags & MADT_LAPIC_FLAG_ONLINE_CAPABLE)
            && !(lapics[i]->flags & MADT_LAPIC_FLAG_ENABLED)) {
            klog_info("CPU %d is not enabled or online capable\n", lapics[i]->proc_id);
            continue;
        }

        // if cpu is the bootstrap processor, do not initialize it
        if (apic_read_reg(APIC_REG_ID) == lapics[i]->apic_id) {
            klog_info("CPU %d is BSP\n", lapics[i]->proc_id);
            info.cpus[info.num_cpus].is_bsp = true;
            info.cpus[info.num_cpus].lapic_id = lapics[i]->apic_id;
            info.num_cpus++;
            continue;
        }

        klog_info("Initializing CPU %d...", lapics[i]->proc_id);

        // send the init ipi
        apic_send_ipi(lapics[i]->apic_id, 0, APIC_IPI_TYPE_INIT);
        pit_wait(10);

        bool success = false;
        // send startup ipi 2 times
        for (int k = 0; k < 2; k++) {
            apic_send_ipi(lapics[i]->apic_id, SMP_TRAMPOLINE_BLOB_ADDR / PAGE_SIZE, APIC_IPI_TYPE_STARTUP);

            // check if cpu has started
            for (int j = 0; j < 20; j++) {
                counter_curr = *ap_boot_counter;

                if (counter_curr != counter_prev) {
                    success = true;
                    break;
                }
                pit_wait(10);
            }
            if (success)
                break;
        }

        if (!success) {
            klog_printf(" Failed\n");
        } else {
            info.cpus[info.num_cpus].is_bsp = false;
            info.cpus[info.num_cpus].lapic_id = lapics[i]->apic_id;
            info.num_cpus++;
            klog_printf(" Done\n");
        }
    }

    klog_ok("SMP initialized. %d processors detected\n", info.num_cpus);
}