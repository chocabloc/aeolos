#pragma once

#include "boot/stivale2.h"
#include <stdint.h>

#define TERM_COLOR_BLACK 0x000000
#define TERM_COLOR_WHITE 0xffffff
#define TERM_COLOR_GRAY 0x808080
#define TERM_COLOR_RED 0xff0000
#define TERM_COLOR_GREEN 0x00ff00
#define TERM_COLOR_BLUE 0x0000ff
#define TERM_COLOR_CYAN 0x00ffff
#define TERM_COLOR_MAGENTA 0xff00ff
#define TERM_COLOR_YELLOW 0xffff00
#define TERM_COLOR_LTRED 0xff6666
#define TERM_COLOR_LTGREEN 0x66ff66
#define TERM_COLOR_LTBLUE 0x6666ff
#define TERM_COLOR_ORANGE 0xffaa66

typedef struct PSF_font {
    uint32_t magic; /* magic bytes to identify PSF */
    uint32_t version; /* zero */
    uint32_t headersize; /* offset of bitmaps in file, 32 */
    uint32_t flags; /* 0 if there's no unicode table */

    uint32_t numglyph; /* number of glyphs */
    uint32_t glyph_size; /* size of each glyph */
    uint32_t height; /* height in pixels */
    uint32_t width; /* width in pixels */

    uint8_t data[]; /* the actual font data */
} PSF_font;

void term_init();

void term_putchar(uint8_t c);
void term_puts(const char* s);
void term_puthex(uint64_t n);
void term_putint(int n);

void term_setfgcolor(uint32_t color);
void term_setbgcolor(uint32_t color);
void term_clear();
void term_flush();
