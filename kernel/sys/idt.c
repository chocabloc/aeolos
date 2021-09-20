#include "idt.h"
#include "isrs.h"
#include "klog.h"
#include "panic.h"
#include "syscall.h"
#include <stdint.h>

static struct idt_entry IDT[256];

// start allocating irq's at SYSCALL_IRQ_VECTOR + 1
static uint8_t lastvector = SYSCALL_IRQ_VECTOR;

extern void idt_load(struct idtr*);

static struct idt_entry idt_make_entry(uint64_t offset, bool usermode)
{
    return (struct idt_entry) {
        .selector = CODE_SEGMENT_SELECTOR,
        .offset_15_0 = offset & 0xFFFF,
        .offset_31_16 = (offset >> 16) & 0xFFFF,
        .offset_63_32 = (offset >> 32) & 0xFFFFFFFF,
        .flags = usermode ? IDT_FLAGS_USER : IDT_FLAGS_DEFAULT
    };
}

void idt_set_handler(uint8_t vector, void* handler, bool usermode)
{
    IDT[vector] = idt_make_entry((uint64_t)handler, usermode);
}

uint8_t idt_get_vector()
{
    lastvector++;
    if (lastvector == 0)
        kernel_panic("Out of IRQ vectors\n");

    return lastvector;
}

void idt_init()
{
    // exception isr's
    IDT[0] = idt_make_entry((uint64_t)&isr0, false);
    IDT[1] = idt_make_entry((uint64_t)&isr1, false);
    IDT[2] = idt_make_entry((uint64_t)&isr2, false);
    IDT[3] = idt_make_entry((uint64_t)&isr3, false);
    IDT[4] = idt_make_entry((uint64_t)&isr4, false);
    IDT[5] = idt_make_entry((uint64_t)&isr5, false);
    IDT[6] = idt_make_entry((uint64_t)&isr6, false);
    IDT[7] = idt_make_entry((uint64_t)&isr7, false);
    IDT[8] = idt_make_entry((uint64_t)&isr8, false);
    IDT[10] = idt_make_entry((uint64_t)&isr10, false);
    IDT[11] = idt_make_entry((uint64_t)&isr11, false);
    IDT[12] = idt_make_entry((uint64_t)&isr12, false);
    IDT[13] = idt_make_entry((uint64_t)&isr13, false);
    IDT[14] = idt_make_entry((uint64_t)&isr14, false);
    IDT[16] = idt_make_entry((uint64_t)&isr16, false);
    IDT[17] = idt_make_entry((uint64_t)&isr17, false);
    IDT[18] = idt_make_entry((uint64_t)&isr18, false);
    IDT[19] = idt_make_entry((uint64_t)&isr19, false);
    IDT[20] = idt_make_entry((uint64_t)&isr20, false);
    IDT[30] = idt_make_entry((uint64_t)&isr30, false);

    // syscall isr
    IDT[SYSCALL_IRQ_VECTOR] = idt_make_entry((uint64_t)&syscall_entry, true);

    struct idtr i = { .limit = sizeof(IDT) - 1, .base = (uint64_t)&IDT };

    idt_load(&i);
    klog_ok("done\n");
}
