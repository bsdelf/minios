#ifndef FONT_H
#define FONT_H

#include <types.h>

#define FONT_WIDTH  8
#define FONT_HEIGHT 16
#define FONT_WMASK  (1<<7)

extern const uint8 FONT_DATA[256][16];

#endif
