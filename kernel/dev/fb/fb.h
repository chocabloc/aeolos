#pragma once

#include "boot/stivale2.h"
#include <stdint.h>

typedef struct {
    uint8_t* addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
} fb_info;

void fb_init(stv2_struct_tag_fb*);
void fb_putpixel(uint32_t x, uint32_t y, uint32_t color);
uint32_t fb_getpixel(uint32_t x, uint32_t y);
const fb_info* fb_getinfo();
void fb_enable_double_buffering();
void fb_swap_buffers();
