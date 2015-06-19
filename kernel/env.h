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
} smap_entry_t; // 24B

typedef struct {
    uint8 cyls;
    uint8 led;

    uint16 lmem;
    uint16 nhmem;
    smap_entry_t ehmem[10];

    uint16 vmode;
    uint16 xpixel;
    uint16 ypixel;
    uint32 video_pa;
    uint32 video_va;
    uint32 video_size;

    uint32 kern_pa;
    uint32 kern_va;
    uint32 kern_entry;
    uint32 kern_size;
    uint32 stack_pa;
    uint32 stack_va;
    uint32 stack_size;
    uint32 dir_pa;
    uint32 dir_va;
} bootinfo_t;
#pragma pack(pop)

void env_init(void* va);
bootinfo_t* env_bootinfo(void);

#endif
