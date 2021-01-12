#include "idt.h"
#include "isrs.h"
#include "panic.h"
#include "klog.h"
#include <stdint.h>

static struct idt_entry IDT[256];

// start allocating irq's at 60
static uint8_t lastvector = 60;

extern void idt_load(struct idtr*);

static struct idt_entry idt_make_entry(uint64_t offset)
{
    return (struct idt_entry) {
        .selector = CODE_SEGMENT_SELECTOR,
        .offset_15_0 = offset & 0xFFFF,
        .offset_31_16 = (offset >> 16) & 0xFFFF,
        .offset_63_32 = (offset >> 32) & 0xFFFFFFFF,
        .flags = IDT_FLAGS_DEFAULT
    };
}

void idt_set_handler(uint8_t vector, void* handler)
{
    IDT[vector] = idt_make_entry((uint64_t)handler);
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
    IDT[0] = idt_make_entry((uint64_t)&isr0);
    IDT[1] = idt_make_entry((uint64_t)&isr1);
    IDT[2] = idt_make_entry((uint64_t)&isr2);
    IDT[3] = idt_make_entry((uint64_t)&isr3);
    IDT[4] = idt_make_entry((uint64_t)&isr4);
    IDT[5] = idt_make_entry((uint64_t)&isr5);
    IDT[6] = idt_make_entry((uint64_t)&isr6);
    IDT[7] = idt_make_entry((uint64_t)&isr7);
    IDT[8] = idt_make_entry((uint64_t)&isr8);
    IDT[10] = idt_make_entry((uint64_t)&isr10);
    IDT[11] = idt_make_entry((uint64_t)&isr11);
    IDT[12] = idt_make_entry((uint64_t)&isr12);
    IDT[13] = idt_make_entry((uint64_t)&isr13);
    IDT[14] = idt_make_entry((uint64_t)&isr14);
    IDT[16] = idt_make_entry((uint64_t)&isr16);
    IDT[17] = idt_make_entry((uint64_t)&isr17);
    IDT[18] = idt_make_entry((uint64_t)&isr18);
    IDT[19] = idt_make_entry((uint64_t)&isr19);
    IDT[20] = idt_make_entry((uint64_t)&isr20);
    IDT[30] = idt_make_entry((uint64_t)&isr30);

    struct idtr i = { .limit = sizeof(IDT) - 1, .base = (uint64_t)&IDT };

    idt_load(&i);
    klog_ok("IDT initialized\n");
}