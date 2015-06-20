#include "env.h"
#include <stand.h>

bootinfo_t bootinfo;

void env_init(void* va)
{
    memcpy(&bootinfo, va, sizeof(bootinfo_t));
}
