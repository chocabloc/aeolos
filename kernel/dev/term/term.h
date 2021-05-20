#pragma once

#include "boot/stivale2.h"
#include <stdbool.h>
#include <stdint.h>

#define TERM_COLOR_BLACK 0x000000
#define TERM_COLOR_RED 0xaa0000
#define TERM_COLOR_GREEN 0x00aa00
#define TERM_COLOR_YELLOW 0xaaaa00
#define TERM_COLOR_BLUE 0x0000aa
#define TERM_COLOR_MAGENTA 0xaa00aa
#define TERM_COLOR_CYAN 0x00aaaa
#define TERM_COLOR_WHITE 0xaaaaaa

#define TERM_COLOR_LTBLACK 0x555555
#define TERM_COLOR_LTRED 0xff5555
#define TERM_COLOR_LTGREEN 0x55ff55
#define TERM_COLOR_LTYELLOW 0xffff55
#define TERM_COLOR_LTBLUE 0x5555ff
#define TERM_COLOR_LTMAGENTA 0xff55ff
#define TERM_COLOR_LTCYAN 0x55ffff
#define TERM_COLOR_LTWHITE 0xffffff

#define DEFAULT_FGCOLOR TERM_COLOR_WHITE
#define DEFAULT_BGCOLOR TERM_COLOR_BLACK

typedef struct {
    uint32_t magic; /* magic bytes to identify PSF */
    uint32_t version; /* zero */
    uint32_t headersize; /* offset of bitmaps in file, 32 */
    uint32_t flags; /* 0 if there's no unicode table */

    uint32_t numglyph; /* number of glyphs */
    uint32_t glyph_size; /* size of each glyph */
    uint32_t height; /* height in pixels */
    uint32_t width; /* width in pixels */

    uint8_t data[]; /* the actual font data */
} psfont_t;

void term_init();
void term_putchar(uint8_t c);
void term_setfgcolor(uint32_t color);
void term_setbgcolor(uint32_t color);
uint32_t term_getwidth();
uint32_t term_getheight();
void term_clear();
void term_flush();
