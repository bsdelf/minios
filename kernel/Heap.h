#ifndef HEAP_H
#define HEAP_H

#include <types.h>

void HeapInit(uint32 addr, uint32 size);

void* Malloc(uint32 size, bool align, uint32* paddr);
void* Calloc(uint32 size, bool align, uint32* paddr);
void Free(void* mem);

uint32 KHeapAvail(void);
uint32 KHeapTotal(void);

void PrintKHeap(void);

#endif
