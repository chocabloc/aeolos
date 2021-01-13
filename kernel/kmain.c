#include "acpi/acpi.h"
#include "dev/fb/fb.h"
#include "dev/term/term.h"
#include "klog.h"
#include "kmalloc.h"
#include "lib/random.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "proc/task.h"
#include "sys/apic/apic.h"
#include "sys/cpu/cpu.h"
#include "sys/smp/smp.h"
#include "sys/gdt.h"
#include "sys/idt.h"
#include <stdbool.h>
#include <stddef.h>

extern uint8_t kernel_start;
extern uint8_t kernel_end;

// first task to be executed
void kinit(tid_t tid)
{
    klog_ok("Multitasking initialized. First kernel task started :)\n");
    *((uint32_t*)0x400000000) = 1234;
    task_destroy(tid);
}

void kmain(stivale2_struct* bootinfo)
{
    // convert the physical address to a virtual one, since we will be removing identity mapping later
    bootinfo = (stivale2_struct*)PHYS_TO_VIRT(bootinfo);

    // initialize gdt and idt
    gdt_init();
    idt_init();

    // initialize pmm and vmm
    pmm_init((stv2_struct_tag_mmap*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_MMAP_ID));
    vmm_init();
    cpu_features_init();

    // initialize framebuffer and terminal
    fb_init((stv2_struct_tag_fb*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_FB_ID));
    term_init();

    // we are finally ready to show the log on the screen
    klog_show();

    // draw the banner
    klog_printf(" \xda\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xbf\n");
    klog_printf(" \xb3   Aeolos v0.1  \xb3\xb1\n \xb3 by Aditya Arsh \xb3\xb1\n");
    klog_printf(" \xc0\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xd9\xb1\n");
    klog_printf("  \xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\n");
    klog_printf("Built on "__DATE__
                " at "__TIME__
                ".\n\n");

    // parse acpi tables
    acpi_init((stv2_struct_tag_rsdp*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_RSDP_ID));
    apic_init();
    smp_init();

    // since we do not need the bootloader info anymore
    pmm_reclaim_bootloader_mem();

    // initialize multitasking
    task_init(kinit);

    // DO NOT put anything here. Put it in kinit() instead

    while (true)
        ;
}
