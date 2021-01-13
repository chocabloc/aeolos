#include "smp.h"
#include "acpi/madt.h"
#include "sys/apic/apic.h"
#include "klog.h"

void smp_init()
{
    klog_warn("smp_init(): STUB\n");

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
        //apic_send_ipi(lapics[i]->apic_id, 0, APIC_IPI_TYPE_STARTUP);
        klog_printf(" Failed\n");
    }
}