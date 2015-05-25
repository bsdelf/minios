#include "panic.h"

#include <stand.h>
#include "screen.h"

void _do_panic_(const char* file, uint32 line, const char* fmt, ...)
{
    __asm__("cli");

    screen_setfg(COLOR_White);
    screen_setbg(COLOR_Blue);
    screen_clear();
    screen_printf("==== panic ====\n"
                  "file: %s\n"
                  "line: %d\n\n",
                  file, line);
    va_list ap;
    va_start(ap, fmt);
    screen_vprintf(fmt, ap);
    va_end(ap);

    __asm__("hlt");
}
