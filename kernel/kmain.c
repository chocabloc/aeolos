#include "acpi/acpi.h"
#include "dev/fb/fb.h"
#include "dev/term/term.h"
#include "kconio.h"
#include "kmalloc.h"
#include "lib/random.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "sys/apic.h"
#include "sys/cpu/cpu.h"
#include "sys/gdt.h"
#include "sys/idt.h"
#include <stdbool.h>
#include <stddef.h>

extern uint8_t kernel_start;
extern uint8_t kernel_end;

void kmain(stivale2_struct* bootinfo)
{
    // convert the physical address to a virtual one, since we will be removing identity mapping later
    bootinfo = (stivale2_struct*)PHYS_TO_VIRT(bootinfo);

    // initialize gdt and idt
    gdt_init();
    idt_init();

    // initialize framebuffer and terminal
    fb_init((stv2_struct_tag_fb*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_FB_ID));
    term_init();
    term_clear();

    // draw the banner
    kprintf(" \xda\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xbf\n");
    kprintf(" \xb3   Aeolos v0.1  \xb3\xb1\n \xb3 by Aditya Arsh \xb3\xb1\n");
    kprintf(" \xc0\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xd9\xb1\n");
    kprintf("  \xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\n\n");

    // initialize pmm and vmm
    pmm_init((stv2_struct_tag_mmap*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_MMAP_ID));
    vmm_init();
    cpu_features_init();
    fb_enable_double_buffering();

    // parse acpi tables
    acpi_init((stv2_struct_tag_rsdp*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_RSDP_ID));

    apic_init();

    // since we do not need the bootloader info anymore
    pmm_reclaim_bootloader_mem();

    kdbg_info("Testing interrupts...\n");
    // testing interrupts by doing a page fault
    uint64_t a = 0x400000000000;
    *((uint32_t*)a) = 2345;

    while (true)
        ;
}
