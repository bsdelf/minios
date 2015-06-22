#include "slab.h"

void uma_init(void)
{
}

uma_zone_t uma_zcreate(const char *name, size_t size,
                       uma_ctor_t ctor, uma_dtor_t dtor,
                       uma_init_t init, uma_fini_t fini,
                       int align, uint32 flags)
{
}

void* uma_malloc(uint32 size)
{
}

void uma_free(void* p)
{
}
