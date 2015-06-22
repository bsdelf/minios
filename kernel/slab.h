#ifndef SLAB_H
#define SLAB_H

#include <types.h>
#include <queue.h>
#include "vm.h"

/* Definitions for align */
#define UMA_ALIGN_PTR   (sizeof(void *) - 1)    /* Alignment fit for ptr */
#define UMA_ALIGN_LONG  (sizeof(long) - 1)  /* "" long */
#define UMA_ALIGN_INT   (sizeof(int) - 1)   /* "" int */
#define UMA_ALIGN_SHORT (sizeof(short) - 1) /* "" short */
#define UMA_ALIGN_CHAR  (sizeof(char) - 1)  /* "" char */
#define UMA_ALIGN_CACHE (0 - 1)         /* Cache line size align */

typedef int (*uma_ctor_t)(void *mem, int size, void *arg, int flags);
typedef void (*uma_dtor_t)(void *mem, int size, void *arg);
typedef int (*uma_init_t)(void *mem, int size, int flags);
typedef void (*uma_fini_t)(void *mem, int size);

typedef struct uma_zone {
} uma_zone_t;

typedef struct uma_slab {
} uma_slab_t;

typedef struct uma_keg {
    const char* name;

    LIST_HEAD(, uma_zone) zones;
    LIST_HEAD(, uma_slab) part_slab;
    LIST_HEAD(, uma_slab) free_slab;
    LIST_HEAD(, uma_slab) full_slab;

    uma_ctor_t ctor;
    uma_dtor_t dtor;
    uma_init_t init;
    uma_fini_t fini;

    vm_object_t* vmo;
} uma_keg_t;

void uma_init(void);

uma_zone_t uma_zcreate(const char *name, size_t size,
                       uma_ctor_t ctor, uma_dtor_t dtor,
                       uma_init_t init, uma_fini_t fini,
                       int align, uint32 flags);

void* uma_malloc(uint32 size);
void uma_free(void* p);

#endif
