#include "dev/fb/fb.h"
#include "dev/term/term.h"
#include "kconio.h"
#include "kmalloc.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "sys/acpi/acpi.h"
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

    gdt_init();
    idt_init();

    fb_init((stv2_struct_tag_fb*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_FB_ID));
    term_init();
    term_clear();

    kprintf("==Aeolos v0.1==\n\n");
    kdbg_ok("GDT initialized\n");
    kdbg_ok("IDT initialized\n");

    pmm_init((stv2_struct_tag_mmap*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_MMAP_ID));
    kdbg_ok("PMM initialized\n");

    kdbg_info("%x %x\n", (uint64_t)fb_getinfo()->addr, fb_getinfo()->pitch * fb_getinfo()->height);

    vmm_init();
    kdbg_ok("VMM initialized\n");

    acpi_init((stv2_struct_tag_rsdp*)stv2_find_struct_tag(bootinfo, STV2_STRUCT_TAG_RSDP_ID));

    kdbg_info("Testing interrupts...\n");
    // testing interrupts by doing a page fault
    uint64_t a = 0x400000000000;
    *((uint32_t*)a) = 2345;

    while (true)
        ;
}
