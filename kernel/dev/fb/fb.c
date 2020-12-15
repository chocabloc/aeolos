#include "fb.h"
#include "mm/vmm.h"

static fb_info fb;

const fb_info* fb_getinfo()
{
    return &fb;
}

void fb_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    ((uint32_t*)(fb.addr + (fb.pitch * y)))[x] = color;
}

uint32_t fb_getpixel(uint32_t x, uint32_t y)
{
    return ((uint32_t*)(fb.addr + (fb.pitch * y)))[x];
}

void fb_init(stv2_struct_tag_fb* t)
{
    fb.addr = (uint8_t*)PHYS_TO_VIRT(t->fb_addr);
    fb.width = t->fb_width;
    fb.height = t->fb_height;
    fb.pitch = t->fb_pitch;
}