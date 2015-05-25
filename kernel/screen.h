#ifndef SCREEN_H
#define SCREEN_H

#include <stand.h>

#include "font.h"
#include "color.h"

#define BLACK   0
#define WHITE   1
#define RED     2
#define LIME    3
#define BLUE    4
#define YELLOW  5

void screen_init(void);
void screen_clear(void);
void screen_erase(int n);
int screen_printf(const char *cfmt, ...);
int screen_vprintf(const char *cfmt, va_list ap);
void screen_setfg(uint8 color);
void screen_setbg(uint8 color);
uint8 screen_getfg(void);
uint8 screen_getbg(void);

int DrawChar(int x, int y, char ch, uint8 fg, uint8 bg);
int DrawString(int x, int y, const char* str, uint8 fg, uint8 bg);
int DrawSigned(int x, int y, int32 num, uint8 fg, uint8 bg);
int DrawUnsigned(int x, int y, uint32 num, uint8 fg, uint8 bg);

#endif
