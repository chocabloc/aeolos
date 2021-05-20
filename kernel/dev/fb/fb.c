#include "fb.h"
#include "klog.h"
#include "kmalloc.h"
#include "memutils.h"
#include "mm/mm.h"
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
    fb.addr = (uint8_t*)PHYS_TO_VIRT(t->fb_addr);
    fb.width = t->fb_width;
    fb.height = t->fb_height;
    fb.pitch = t->fb_pitch;
    klog_info("framebuffer resolution is %dx%d\n", fb.width, fb.height);

    /* mapping the framebuffer
     * also use write-combining cache
    */
    uint64_t fbsize = NUM_PAGES(fb.pitch * fb.height);
    vmm_map(NULL, (uint64_t)fb.addr, VIRT_TO_PHYS(fb.addr), fbsize, VMM_FLAGS_DEFAULT | VMM_FLAG_WRITECOMBINE);

    // initialize double buffering
    backbuffer = kmalloc(fb.pitch * fb.height);
    klog_info("done\n");
}

// swap back and front buffers
void fb_swap_buffers()
{
    memcpy(backbuffer, fb.addr, fb.pitch * fb.height);
}
