#ifndef ENV_H
#define ENV_H

#include <types.h>

#pragma pack(push, 1)
typedef struct {
    uint32 basel;
    uint32 baseh;
    uint32 lengthl;
    uint32 lengthh;
    uint16 type;
    uint16 acpi;
    uint32 padding;
} SmapEntry; // 24B

typedef struct {
    uint8 cyls;
    uint8 led;

    uint16 lmem;
    uint16 nhmem;
    SmapEntry ehmem[10];

    uint16 vmode;
    uint16 xpixel;
    uint16 ypixel;
    uint32 vram;

    uint32 kern_pa;
    uint32 kern_va;
    uint32 kern_entry;
    uint32 kern_size;
    uint32 stack_pa;
    uint32 stack_va;
    uint32 stack_size;
    uint32 dir_pa;
    uint32 dir_va;
} env_t;
#pragma pack(pop)

void env_init(void* addr);
env_t* env_get(void);

#endif
