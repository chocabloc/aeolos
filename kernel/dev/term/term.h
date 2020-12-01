#pragma once

#include "boot/stivale2.h"
#include <stdint.h>

#define TERM_COLOR_BLACK 0x00000000
#define TERM_COLOR_WHITE 0x00FFFFFF
#define TERM_COLOR_GRAY 0x00888888
#define TERM_COLOR_RED 0x00FF0000
#define TERM_COLOR_GREEN 0x0000FF00
#define TERM_COLOR_BLUE 0x000000FF
#define TERM_COLOR_CYAN 0x0000FFFF
#define TERM_COLOR_MAGENTA 0x00FF00FF
#define TERM_COLOR_YELLOW 0x00FFFF00

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
