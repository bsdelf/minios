#ifndef PANIC_H
#define PANIC_H

#include <types.h>

#define panic(vas...) _do_panic_(__FILE__, __LINE__, vas);

void _do_panic_(const char* file, uint32 line, const char* fmt, ...);

#endif
