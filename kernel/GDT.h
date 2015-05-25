#ifndef GDT_H
#define GDT_H

#include <types.h>

#pragma pack(push, 1)

typedef struct {
    uint16 limit;
    uint32 base;
} GdtPtr;

#pragma pack(pop)

void InitGDT(void);
void EnableUserMode(void);
void SetKernelStack(uint32 paddr);

#endif
