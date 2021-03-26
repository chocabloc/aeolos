#include "stivale2.h"
#include "klog.h"
#include "mm/vmm.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static uint8_t kernel_stack[16384] = { 0 };

// framebuffer request tag for bootloader
static stv2_hdr_tag_fb header_fb_tag = {
    .tag = { .identifier = STV2_HDR_TAG_FB_ID, .next = 0 },
    .fb_width = 800,
    .fb_height = 600,
    .fb_bpp = 32
};

// stivale2 header
__attribute__((section(".stivale2hdr"), used)) static stv2_hdr header = {
    .entry_point = 0,
    .stack = (uintptr_t)kernel_stack + sizeof(kernel_stack),
    .flags = 0,
    .tags = (uint64_t)&header_fb_tag
};

// find tag with a specific ID in a stivale2 structure
void* stv2_find_struct_tag(stivale2_struct* s, uint64_t id)
{
    for (void* t = s->tags; t;) {
        stv2_tag* tag = (stv2_tag*)PHYS_TO_VIRT(t);
        if (tag->identifier == id)
            return tag;
        t = tag->next;
    }

    klog_warn("stv2_find_struct_tag(): tag %x not found\n", id);
    return NULL;
}
