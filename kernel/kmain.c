#include "dev/fb/fb.h"
#include "dev/term/term.h"
#include "kconio.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "sys/gdt.h"
#include "sys/idt.h"
#include <stdbool.h>
#include <stddef.h>

extern uint8_t kernel_start;
extern uint8_t kernel_end;

void kmain(stivale2_struct* bootinfo)
{
    gdt_init();
    idt_init();

    // parse the information provided by bootloader
    for (stv2_tag* t = bootinfo->tags; t; t = t->next) {
        switch (t->identifier) {

        case STV2_STRUCT_TAG_FB_ID:
            fb_init((stv2_struct_tag_fb*)t);
            term_init();
            term_clear();
            kprintf("Welcome to Aditya Arsh's OS\n\n");
            kdbg_ok("Framebuffer console initialized\n");
            kdbg_ok("GDT initialized\n");
            kdbg_ok("IDT initialized\n");
            break;

        case STV2_STRUCT_TAG_MMAP_ID:
            pmm_init((stv2_struct_tag_mmap*)t);
            kdbg_ok("PMM initialized\n");
            break;
        }
    }

    vmm_init();
    kdbg_ok("VMM initialized\n");

    kdbg_info("Kernel start: %x\n", (uint64_t)&kernel_start);
    kdbg_info("Kernel end: %x\n", (uint64_t)&kernel_end);

    kprintf("Testing interrupts...\n");

    // testing interrupts by doing a page fault
    uint64_t a = 0x400000000000;
    *((uint32_t*)a) = 2345;

    while (true)
        ;
}
