#ifndef HEAP_H
#define HEAP_H

#include <types.h>

void    heap_init       (uint32 va, uint32 size);
void*   heap_alloc      (uint32 size);
void*   heap_calloc     (uint32 size);
void    heap_release    (void* va);
uint32  heap_avail      (void);
uint32  heap_occup      (void);
void    heap_status     (void);

#endif
