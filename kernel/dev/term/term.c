#include "term.h"
#include "boot/stivale2.h"
#include "dev/fb/fb.h"
#include "klog.h"
#include "memutils.h"
#include <stddef.h>

extern psfont_t term_font;

static const fb_info* fb;
static const uint32_t ansicolors[16] = {
    TERM_COLOR_BLACK, TERM_COLOR_RED, TERM_COLOR_GREEN, TERM_COLOR_YELLOW,
    TERM_COLOR_BLUE, TERM_COLOR_MAGENTA, TERM_COLOR_CYAN, TERM_COLOR_WHITE,
    TERM_COLOR_LTBLACK, TERM_COLOR_LTRED, TERM_COLOR_LTGREEN, TERM_COLOR_LTYELLOW,
    TERM_COLOR_LTBLUE, TERM_COLOR_LTMAGENTA, TERM_COLOR_LTCYAN, TERM_COLOR_LTWHITE
};

static uint32_t fgcolor = TERM_COLOR_WHITE;
static uint32_t bgcolor = TERM_COLOR_BLACK;

// x and y position of cursor, measured in characters not pixels
static uint32_t cursor_x, cursor_y;

// width and height of framebuffer in characters
static uint32_t term_width, term_height;

// is the terminal ready
static bool ready = false;

// implement ansi color escape sequences
static bool parse_cmd_byte(uint8_t byte)
{
    static enum {
        READY,
        CMD_WAIT,
        PARAM_WAIT
    } state;
    static int cparams[10] = { 0 };
    static int cparamcount = 0;

    if (state == READY) {
        if (byte == 0x1B)
            state = CMD_WAIT;
        else
            goto err;
    } else if (state == CMD_WAIT) {
        if (byte == '[') {
            cparamcount = 1;
            cparams[0] = 0;
            state = PARAM_WAIT;
        } else if (byte == 'c') {
            term_clear();
            goto success;
        } else
            goto err;
    } else if (state == PARAM_WAIT) {
        if (byte == ';') {
            cparams[cparamcount++] = 0;
        } else if (byte == 'm') {
            if (cparams[0] == 0) {
                term_setfgcolor(TERM_COLOR_WHITE);
                term_setbgcolor(TERM_COLOR_BLACK);
            } else if (cparams[0] >= 30 && cparams[0] <= 37) {
                if (cparamcount == 2 && cparams[1] == 1) {
                    term_setfgcolor(ansicolors[cparams[0] - 30 + 8]);
                } else {
                    term_setfgcolor(ansicolors[cparams[0] - 30]);
                }
            } else if (cparams[0] >= 40 && cparams[0] <= 47) {
                if (cparamcount == 2 && cparams[1] == 1) {
                    term_setbgcolor(ansicolors[cparams[0] - 40 + 8]);
                } else {
                    term_setbgcolor(ansicolors[cparams[0] - 40]);
                }
            }
            goto success;
        } else if (byte >= '0' && byte <= '9') {
            cparams[cparamcount - 1] *= 10;
            cparams[cparamcount - 1] += byte - '0';
        } else {
            goto err;
        }
    } else {
        goto err;
    }
    return true;

err:
    state = READY;
    cparamcount = 0;
    return false;

success:
    state = READY;
    cparamcount = 0;
    return true;
}

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
            fb_putpixel(x + j, y + i, (glyph[i] & masks[j]) ? fgcolor : bgcolor);
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
    if (parse_cmd_byte(c))
        return;

    switch (c) {
    case '\0':
        return;

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

// initialize the terminal
void term_init()
{
    fb = fb_getinfo();

    term_width = fb->width / term_font.width;
    term_height = fb->height / term_font.height;

    term_clear();
    term_flush();

    ready = true;
}

void term_setfgcolor(uint32_t color)
{
    fgcolor = color;
}

void term_setbgcolor(uint32_t color)
{
    bgcolor = color;
}

bool term_isready()
{
    return ready;
}

uint32_t term_getwidth()
{
    return term_width;
}

uint32_t term_getheight()
{
    return term_height;
}