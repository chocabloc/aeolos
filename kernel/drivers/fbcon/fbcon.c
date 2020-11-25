#include "fbcon.h"
#include "boot/stivale2.h"
#include "stddef.h"

extern PSF_font fbcon_font;

static uint32_t scanline;
static uint32_t width;
static uint32_t height;
static uint8_t* framebuffer;

static uint32_t fgcolor = FBCON_COLOR_GRAY;
static uint32_t bgcolor = FBCON_COLOR_BLACK;

// x and y position of cursor, measured in characters not pixels
static uint32_t cursor_x, cursor_y;

// width and height of framebuffer in characters
static uint32_t fbcon_width, fbcon_height;

// put pixel at specified x and y coordinates
static inline void putpixel(size_t x, size_t y, uint32_t color)
{
    ((uint32_t*)(framebuffer + (scanline * y)))[x] = color;
}

void fbcon_setfgcolor(uint32_t color) { fgcolor = color; }
void fbcon_setbgcolor(uint32_t color) { bgcolor = color; }

// scroll the screen one line up
static void scroll()
{
    for (size_t i = 0; i < scanline * height; i++)
        *(framebuffer + i) = *(framebuffer + i + (scanline * fbcon_font.height));
}

// clear the screen with the background color
void fbcon_clear()
{
    for (size_t i = 0; i < (scanline / 4) * height; i++) {
        *((uint32_t*)framebuffer + i) = bgcolor;
    }
    cursor_x = 0;
    cursor_y = 0;
}

// put character at specified x and y position, measured in characters
static void putchar_at(uint8_t c, size_t px, size_t py)
{
    uint8_t* glyph = &fbcon_font.data[c * fbcon_font.glyph_size];

    // calculate x and y position in pixels
    size_t x = px * fbcon_font.width, y = py * fbcon_font.height;

    static const uint8_t masks[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

    for (size_t i = 0; i < fbcon_font.height; i++) {
        for (size_t j = 0; j < fbcon_font.width; j++) {
            if (glyph[i] & masks[j])
                putpixel(x + j, y + i, fgcolor);
            else
                putpixel(x + j, y + i, bgcolor);
        }
    }
}

void fbcon_putchar(uint8_t c)
{
    switch (c) {
    case '\n':
        cursor_x = 0;
        cursor_y++;
        break;

    case '\t':
        cursor_x += (cursor_x % 4 == 0) ? 0 : (4 - cursor_x % 4);
        break;

    default:
        putchar_at(c, cursor_x, cursor_y);
        cursor_x++;
    }

    if (cursor_x >= fbcon_width) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= fbcon_height) {
        scroll();
        cursor_y = fbcon_height - 1;
    }
}

void fbcon_puts(const char* s)
{
    for (size_t i = 0; s[i]; i++)
        fbcon_putchar(s[i]);
}

// print number as 64-bit hex
void fbcon_puthex(uint64_t n)
{
    fbcon_puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        uint64_t digit = (n >> i) & 0xF;
        fbcon_putchar((digit <= 9) ? (digit + '0') : (digit - 10 + 'A'));
    }
}

// print number
void fbcon_putint(int n)
{
    if (n == 0) {
        fbcon_putchar('0');
        return;
    }
    if (n < 0) {
        fbcon_putchar('-');
        n = -n;
    }
    size_t div = 1;
    int temp = n;
    while (temp > 0) {
        temp /= 10;
        div *= 10;
    }
    while (div >= 10) {
        int digit = ((n % div) - (n % (div / 10))) / (div / 10);
        div /= 10;
        fbcon_putchar(digit + '0');
    }
}

// initialize the framebuffer console
void fbcon_init(stv2_struct_tag_fb* f)
{
    framebuffer = f->fb_addr;
    width = f->fb_width;
    height = f->fb_height;
    scanline = f->fb_pitch;

    fbcon_width = width / fbcon_font.width;
    fbcon_height = height / fbcon_font.height;
}

uint32_t* fbcon_getframebuffer() { return (uint32_t*)framebuffer; }
