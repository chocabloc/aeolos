#include "fb.h"
#include "kmalloc.h"
#include "memutils.h"
#include "mm/vmm.h"
#include <stdbool.h>

static fb_info fb;

static bool isdoublebuffered;
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

void fb_init(stv2_struct_tag_fb* t)
{
    fb.addr = (uint8_t*)PHYS_TO_VIRT(t->fb_addr);
    fb.width = t->fb_width;
    fb.height = t->fb_height;
    fb.pitch = t->fb_pitch;

    // no double buffering, the back and front buffers are the same
    isdoublebuffered = false;
    backbuffer = fb.addr;
}

void fb_enable_double_buffering()
{
    isdoublebuffered = true;
    backbuffer = kmalloc(fb.pitch * fb.height);
    memcpy_fast(fb.addr, backbuffer, fb.pitch * fb.height);
}

void fb_swap_buffers()
{
    if (isdoublebuffered)
        memcpy_fast(backbuffer, fb.addr, fb.pitch * fb.height);
}