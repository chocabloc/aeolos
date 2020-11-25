#include "drivers/fbcon/fbcon.h"
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

        // if framebuffer tag found, initialize framebuffer console and print welcome message
        case STV2_STRUCT_TAG_FB_ID:
            fbcon_init((stv2_struct_tag_fb*)t);
            fbcon_clear();
            kprintf("Welcome to Aditya Arsh's OS\n\n");
            kdbg_ok("Framebuffer console initialized\n");
            break;

        // if memory map found, initialize pmm
        case STV2_STRUCT_TAG_MMAP_ID:
            pmm_init((stv2_struct_tag_mmap*)t);
            kdbg_ok("PMM initialized\n");
            break;
        }
    }
    vmm_init();

    kdbg_info("Kernel start: %x\n", (uint64_t)&kernel_start);
    kdbg_info("Kernel end: %x\n", (uint64_t)&kernel_end);

    kprintf("Testing interrupts...");

    // testing interrupts by doing a page fault
    uint64_t a = 0x400000000000;
    *((uint32_t*)a) = 2345;

    while (true)
        ;
}
