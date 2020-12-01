#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dev/term/term.h"
#include "stivale2.h"

extern void kmain(stivale2_struct* bootinfo);

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
    .entry_point = (uint64_t)&kmain,
    .stack = (uintptr_t)kernel_stack + sizeof(kernel_stack),
    .flags = 0,
    .tags = (uint64_t)&header_fb_tag
};
