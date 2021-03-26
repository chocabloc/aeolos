#include "fb.h"
#include "klog.h"
#include "kmalloc.h"
#include "memutils.h"
#include "mm/mm.h"
#include <stdbool.h>
#include <stddef.h>

static fb_info fb;

static uint8_t* backbuffer;

const fb_info* fb_getinfo()
{
    return &fb;
}

void fb_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    ((uint32_t*)(backbuffer + (fb.pitch * y)))[x] = color;
}

uint32_t fb_getpixel(uint32_t x, uint32_t y)
{
    return ((uint32_t*)(backbuffer + (fb.pitch * y)))[x];
}

// initialize framebuffer
void fb_init(stv2_struct_tag_fb* t)
{
    klog_info("fb_init(): framebuffer at %x (%dx%d)\n", fb.addr, fb.width, fb.height);
    fb.addr = (uint8_t*)PHYS_TO_VIRT(t->fb_addr);
    fb.width = t->fb_width;
    fb.height = t->fb_height;
    fb.pitch = t->fb_pitch;

    /* mapping the framebuffer
     * map a bit more than height so that scrolling works properly
     * also use write-combining cache (using PAT)
    */
    uint64_t fbsize = NUM_PAGES(fb.pitch * (fb.height + 16));
    vmm_map(NULL, (uint64_t)fb.addr, VIRT_TO_PHYS(fb.addr), fbsize, VMM_FLAGS_DEFAULT | VMM_FLAG_WRITECOMBINE);

    // initialize double buffering
    backbuffer = kmalloc(fb.pitch * fb.height);
}

// swap back and front buffers
void fb_swap_buffers()
{
    memcpy(backbuffer, fb.addr, fb.pitch * fb.height);
}
