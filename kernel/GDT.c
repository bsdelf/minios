#include "GDT.h"
#include "Segments.h"
#include "tss.h"
#include <stand.h>

/* load gdt pointer and flush pipeline(far jump) */
extern void LoadGdt(uint32 gdtptr, uint32 csel, uint32 dsel);

/* load tss by selector */
extern void LoadTss(uint32 tsel);

static i386tss _tss;

static SegmentDescriptor _gdtEntries[] = {
    //base, limit, type, code&data, dpl, present, granularity
    INIT_SEGMENT_DESCRIPTOR(0, 0,       0,   0, 0, 0, 0),
    INIT_SEGMENT_DESCRIPTOR(0, 0xfffff, 0xa, 1, 0, 1, 1), // 0x08 sys code
    INIT_SEGMENT_DESCRIPTOR(0, 0xfffff, 0x2, 1, 0, 1, 1), // 0x10 sys data
    INIT_SEGMENT_DESCRIPTOR(0, 0xfffff, 0xa, 1, 3, 1, 1), // 0x18 usr code
    INIT_SEGMENT_DESCRIPTOR(0, 0xfffff, 0x2, 1, 3, 1, 1), // 0x20 usr data
    INIT_SEGMENT_DESCRIPTOR(0, 0xfffff, 0x9, 0, 3, 1, 1)  // 0x28 usr tss
};

static GdtPtr _gdtPtr = {
    .limit = sizeof(_gdtEntries) - 1,
    .base = (uint32)&_gdtEntries
};

void InitGDT(void)
{
    // correct tss descriptor's range
    {
        const uint32 base = (uint32)&_tss;
        const uint32 limit = base + sizeof(i386tss);
        _gdtEntries[5].limit_low = limit & 0xffff;
        _gdtEntries[5].limit_high = (limit >> 16) & 0xf;
        _gdtEntries[5].base_low = base & 0xffffff;
        _gdtEntries[5].base_high = (base >> 24) & 0xff;
    }

    // init tss content
    bzero(&_tss, sizeof(i386tss));

    _tss.ss0 = 0x10;
    _tss.esp0 = 1*1024*1024*21;

    //_tss.cs = 0x0b;
    //_tss.ss = _tss.ds = _tss.es = _tss.fs = _tss.gs = 0x13;

    // flush all
    LoadGdt((uint32)&_gdtPtr, 0x8, 0x10);
    LoadTss(0x2b);
}

void SetKernelStack(uint32 paddr)
{
    _tss.esp0 = paddr;
}
