#include "term.h"
#include "boot/stivale2.h"
#include "dev/fb/fb.h"
#include "kconio.h"
#include "memutils.h"
#include <stddef.h>

extern PSF_font term_font;

static const fb_info* fb;

static uint32_t fgcolor = TERM_COLOR_GRAY;
static uint32_t bgcolor = TERM_COLOR_BLACK;

// x and y position of cursor, measured in characters not pixels
static uint32_t cursor_x, cursor_y;

// width and height of framebuffer in characters
static uint32_t term_width, term_height;

// scroll the screen one line up
static void scroll()
{
    for (size_t y = 0; y < fb->height - term_font.height; y++)
        for (size_t x = 0; x < fb->width; x++)
            fb_putpixel(x, y, fb_getpixel(x, y + term_font.height));

    for (size_t y = fb->height - term_font.height; y < fb->height; y++)
        for (size_t x = 0; x < fb->width; x++)
            fb_putpixel(x, y, bgcolor);
}

// put character at specified x and y position, measured in characters
static void putchar_at(uint8_t c, size_t px, size_t py)
{
    uint8_t* glyph = &term_font.data[c * term_font.glyph_size];

    // calculate x and y position in pixels
    size_t x = px * term_font.width, y = py * term_font.height;

    static const uint8_t masks[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

    for (size_t i = 0; i < term_font.height; i++) {
        for (size_t j = 0; j < term_font.width; j++) {
            if (glyph[i] & masks[j])
                fb_putpixel(x + j, y + i, fgcolor);
            else
                fb_putpixel(x + j, y + i, bgcolor);
        }
    }
}

// update the terminal
void term_flush()
{
    fb_swap_buffers();
}

// clear the screen with the background color
void term_clear()
{
    for (size_t y = 0; y < fb->height; y++)
        for (size_t x = 0; x < fb->width; x++)
            fb_putpixel(x, y, bgcolor);

    cursor_x = 0;
    cursor_y = 0;
}

void term_putchar(uint8_t c)
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

    if (cursor_x >= term_width) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= term_height) {
        scroll();
        cursor_y = term_height - 1;
    }
}

void term_puts(const char* s)
{
    for (size_t i = 0; s[i]; i++)
        term_putchar(s[i]);
}

// print number as 64-bit hex
void term_puthex(uint64_t n)
{
    term_puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        uint64_t digit = (n >> i) & 0xF;
        term_putchar((digit <= 9) ? (digit + '0') : (digit - 10 + 'A'));
    }
}

// print number
void term_putint(int n)
{
    if (n == 0) {
        term_putchar('0');
        return;
    }
    if (n < 0) {
        term_putchar('-');
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
        term_putchar(digit + '0');
    }
}

// initialize the terminal
void term_init()
{
    fb = fb_getinfo();

    term_width = fb->width / term_font.width;
    term_height = fb->height / term_font.height;
}

void term_setfgcolor(uint32_t color)
{
    fgcolor = color;
}

void term_setbgcolor(uint32_t color)
{
    bgcolor = color;
}