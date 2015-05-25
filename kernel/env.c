#include "env.h"
#include <stand.h>

static env_t _env;

void env_init(void* addr)
{
    memcpy(&_env, addr, sizeof(env_t));
}

env_t* env_get(void)
{
    return &_env;
}
