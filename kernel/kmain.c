#include "dev/fb/fb.h"
#include "dev/serial/serial.h"
#include "dev/term/term.h"
#include "klog.h"
#include "mm/mm.h"
#include "proc/sched/sched.h"
#include "sys/acpi/acpi.h"
#include "sys/apic/apic.h"
#include "sys/cpu/cpu.h"
#include "sys/gdt.h"
#include "sys/hpet.h"
#include "sys/idt.h"
#include "sys/panic.h"
#include "sys/smp/smp.h"
#include <stdbool.h>
#include <stddef.h>

// first task to be executed
void kinit(tid_t tid)
{
    klog_show();
    klog_ok("Multitasking initialized. First kernel task with tid %d started :)\n", tid);
    kernel_panic("This OS is a work in progress\n");
    while (true)
        ;
}

void kmain(stivale2_struct* bootinfo)
{
    // convert the physical address to a virtual one, since we will be removing identity mapping later
    bootinfo = (stivale2_struct*)PHYS_TO_VIRT(bootinfo);

    // draw the banner
    klog_printf(" \xda\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xbf\n");
    klog_printf(" \xb3   Aeolos v0.1  \xb3\xb1\n \xb3  by chocabloc  \xb3\xb1\n");
    klog_printf(" \xc0\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xd9\xb1\n");
    klog_printf("  \xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\n");
    klog_printf("Built on "__DATE__
                " at "__TIME__
                ".\n\n");

    // initialize gdt and idt
    gdt_init();
    idt_init();
    cpu_features_init();

    // initialize pmm and vmm
    pmm_init((stv2_struct_tag_mmap*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_MMAP_ID));
    vmm_init();

    // initialize framebuffer and terminal
    fb_init((stv2_struct_tag_fb*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_FB_ID));
    serial_init();
    term_init();

    // parse acpi tables
    acpi_init((stv2_struct_tag_rsdp*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_RSDP_ID));
    hpet_init();
    apic_init();
    smp_init();

    // since we do not need the bootloader info anymore
    pmm_reclaim_bootloader_mem();

    // initialize multitasking
    sched_init(kinit);

    // DO NOT put anything here. Put it in kinit() instead

    while (true)
        ;
}
