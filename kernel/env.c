#include "env.h"
#include <stand.h>

static bootinfo_t _bootinfo;

void env_init(void* va)
{
    memcpy(&_bootinfo, va, sizeof(bootinfo_t));
}

bootinfo_t* env_bootinfo(void)
{
    return &_bootinfo;
}
