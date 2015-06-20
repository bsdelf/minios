#include "screen.h"
#include "env.h"
#include "asm.h"

static struct {
    uint32 ram;
    int w;
    int h;
    int row_max;
    int col_max;
    int row;
    int col;
    uint8 fg;
    uint8 bg;
} _ctx;

void screen_init(void)
{
    int cmd0 = 0x03c8;
    int cmd1 = 0x03c9;
    outb(cmd0, 0);
    for (int i = 0; i < COLOR_COUNT; ++i) {
        outb(cmd1, RGB_TABLE[i][0]/4);
        outb(cmd1, RGB_TABLE[i][1]/4);
        outb(cmd1, RGB_TABLE[i][2]/4);
    }

    _ctx.ram = bootinfo.video_va;
    _ctx.w = bootinfo.xpixel;
    _ctx.h = bootinfo.ypixel;
    _ctx.col = 0;
    _ctx.row = 0;
    _ctx.col_max = _ctx.w / FONT_WIDTH;
    _ctx.row_max = _ctx.h / FONT_HEIGHT;
    _ctx.fg = COLOR_White;
    _ctx.bg = COLOR_Black;
}

void screen_clear(void)
{
    void* start = (void*)_ctx.ram;
    int count = _ctx.w * _ctx.h;
    memset(start, _ctx.bg, count);
    _ctx.col = 0;
    _ctx.row = 0;
}

void screen_erase(int n)
{
}

void putch(int ch)
{
    int x = _ctx.col * FONT_WIDTH;
    int y = _ctx.row * FONT_HEIGHT;

    if (ch == '\n') {
        _ctx.col = 0;
        _ctx.row++;
    } else if (ch == '\t') {
        _ctx.col += 4;
    } else {
        for (int row = 0; row < FONT_HEIGHT; ++row) {
            uint8 fr = FONT_DATA[(int)ch][row];
            uint8* addr = (uint8*)(_ctx.ram + (y+row)*_ctx.w + x);
            for (int col = 0, mask = FONT_WMASK; col < FONT_WIDTH; ++col) {
                addr[col] = (fr & mask) ? _ctx.fg : _ctx.bg;
                mask >>= 1;
            }
        }
        _ctx.col++;
    }

    if (_ctx.col >= _ctx.col_max) {
        _ctx.col = 0;
        _ctx.row++;
    }
    if (_ctx.row >= _ctx.row_max) {
        void* start = (void*)_ctx.ram;
        {
            int yoff = FONT_HEIGHT * _ctx.w;
            int count = _ctx.w * (_ctx.h - FONT_HEIGHT);
            memcpy(start, start + yoff, count);
        }
        {
            int yoff = _ctx.w * ((_ctx.row_max - 1) * FONT_HEIGHT);
            int count = _ctx.w * _ctx.h - yoff;
            memset(start + yoff, _ctx.bg, count);
        }
        _ctx.row--;
    }
}

int screen_printf(const char *cfmt, ...)
{
    va_list ap;
    int ret;
    va_start(ap, cfmt);
    ret = vcprintf(putch, cfmt, ap);
    va_end(ap);
    return ret;
}

int screen_vprintf(const char *cfmt, va_list ap)
{
    int ret;
    ret = vcprintf(putch, cfmt, ap);
    return ret;
}

void screen_setfg(uint8 color)
{
    _ctx.fg = color;
}

void screen_setbg(uint8 color)
{
    _ctx.bg = color;
}

uint8 screen_getfg(void)
{
    return _ctx.fg;
}

uint8 screen_getbg(void)
{
    return _ctx.bg;
}

int DrawChar(int x, int y, char ch, uint8 fg, uint8 bg)
{
    for (int row = 0; row < FONT_HEIGHT; ++row) {
        uint8 font = FONT_DATA[(int)ch][row];
        uint8* addr = (uint8*)(_ctx.ram + (y+row)*_ctx.w + x);
        for (int col = 0, mask = FONT_WMASK; col < FONT_WIDTH; ++col) {
            addr[col] = (font & mask) ? fg : bg;
            mask >>= 1;
        }
    }
    return FONT_WIDTH;
}

int DrawString(int x, int y, const char* str, uint8 fg, uint8 bg)
{
    int pixel = 0;
    for (; *str != '\0'; ++str) {
        DrawChar(x, y, *str, fg, bg);
        x += FONT_WIDTH;
        pixel += FONT_WIDTH;
    }
    return pixel;
}

int DrawSigned(int x, int y, int32 num, uint8 fg, uint8 bg)
{
    int pixel = 0;
    int width = 1;
    char sign = 0;

    if (num < 0) {
        sign = 1;
        num = -num;
    }

    for (uint32 n = num; n >= 10; n/=10, width*=10);
    
    if (sign == 1) {
        DrawChar(x, y, '-', fg, bg);
        x += FONT_WIDTH;
        pixel += FONT_WIDTH;
    }

    do {
        uint32 nth = num/width;
        DrawChar(x, y, nth+48, fg, bg); // num+48 to num's ascii
        num -= nth*width;
        width /= 10;
        x += FONT_WIDTH;
        pixel += FONT_WIDTH;
    } while (width >= 1);

    return pixel;
}

int DrawUnsigned(int x, int y, uint32 num, uint8 fg, uint8 bg)
{
    int pixel = 0;
    uint32 width = 1;

    for (uint32 n = num; n >= 10; n/=10, width*=10);

    do {
        uint32 nth = num/width;
        DrawChar(x, y, nth+48, fg, bg); // num+48 to num's ascii
        num -= nth*width;
        width /= 10;
        x += FONT_WIDTH;
        pixel += FONT_WIDTH;
    } while (width >= 1);
    
    return pixel;
}
