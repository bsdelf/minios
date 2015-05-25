#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <types.h>

#define __packed__ __attribute__ ((__packed__))

#pragma pack(push, 1)

/*
 * selectors
 */

#define ISPL(s) ((s)&3) /* what is the priority level of a selector */
#define SEL_KPL 0       /* kernel priority level */
#define SEL_UPL 3       /* user priority level */
#define ISLDT(s)    ((s)&SEL_LDT)   /* is it local or global */
#define IDXSEL(s)   (((s)>>3) & 0x1fff)     /* index of selector */ 
#define LSEL(s,r)   (((s)<<3) | SEL_LDT | r)    /* a local selector */
#define GSEL(s,r)   (((s)<<3) | r)          /* a global selector */ 

typedef struct {
    uint32 limit_low      : 16;   /* segment extent (lsb) */
    uint32 base_low       : 24;   /* segment base address (lsb) */
    uint32 type           : 4;    /* segment type */
    uint32 code_data      : 1;    /* code/data(1) or special system segments(0) */
    uint32 dpl            : 2;    /* segment descriptor priority level */
    uint32 present        : 1;    /* segment descriptor present */
    uint32 limit_high     : 4;    /* segment extent (msb) */
    uint32 unused         : 2;    /* unused */
    uint32 def32          : 1;    /* default 32 vs 16 bit size */
    uint32 granularity    : 1;    /* limit granularity (byte/page units)*/
    uint32 base_high      : 8;    /* segment base address  (msb) */
} SegmentDescriptor;

#define INIT_SEGMENT_DESCRIPTOR(base, limit, _type, _code_data, _dpl, _present, _granularity) \
{                                       \
    .limit_low = limit & 0xffff,        \
    .limit_high = (limit >> 16) & 0xf,  \
    .base_low = base & 0xffffff,        \
    .base_high = (base >> 24) & 0xff,   \
    .type = _type,                      \
    .code_data = _code_data,            \
    .dpl = _dpl,                        \
    .present = _present,                \
    .unused = 0,                        \
    .def32 = 1,                         \
    .granularity = _granularity         \
}

#define USD_GET_BASE(sd)     (((sd)->base_low) | (sd)->base_high << 24) 
#define USD_SET_BASE(sd, b)\
    (sd)->base_low = (b);  \
    (sd)->base_high = ((b) >> 24);
#define USD_GET_LIMIT(sd)    (((sd)->limit_low) | (sd)->limit_high << 16)
#define USD_SET_LIMIT(sd, l)\
    (sd)->limit_low = (l); \
    (sd)->limit_high = ((l) >> 16);

/*
 * Gate descriptors (e.g. indirect descriptors)
 *
 * 0~63 bits
 * | 16      | 16       | 8      | 4    | 1       | 2   | 1       | 16      |
 * | offsetL | selector | unused | type | segment | DPL | present | offsetH |
 *
 * offset:
 * interrupt function's offset
 *
 * selector:
 * selector of interrupt function (according to GDT)
 *
 * type:
 * 0101 0x5 32-bit task gate
 * 0110 0x6 16-bit interrupt gate
 * 0111 0x7 16-bit trap gate
 * 1110 0xe 32-bit interrupt gate
 * 1111 0xf 32-bit trap gate
 *
 * segment:
 * = 0 for interrupt gates
 *
 * DPL:
 * descriptor privilege level
 *
 * present:
 * = 0 for unused interrupts or paging
 */

typedef struct {
    uint32 off_low  : 16; /* gate offset (lsb) */
    uint32 selector : 16; /* gate segment selector */
    uint32 unused   : 8;  /* unused */
    uint32 type     : 4;  /* segment type */
    uint32 s        : 1;  /* storage segment (=0 for interrupt gate)*/
    uint32 dpl      : 2;  /* segment descriptor priority level */
    uint32 present  : 1;  /* segment descriptor present */
    uint32 off_high : 16; /* gate offset (msb) */
} GateDescriptor;

#define TASK_GATE   0x5
#define INT_GATE    0xe
#define TRAP_GATE   0xf

/* system segments and gate types */
#define SDT_SYSNULL  0  /* system null */
#define SDT_SYS286TSS    1  /* system 286 TSS available */
#define SDT_SYSLDT   2  /* system local descriptor table */
#define SDT_SYS286BSY    3  /* system 286 TSS busy */
#define SDT_SYS286CGT    4  /* system 286 call gate */
#define SDT_SYSTASKGT    5  /* system task gate */
#define SDT_SYS286IGT    6  /* system 286 interrupt gate */
#define SDT_SYS286TGT    7  /* system 286 trap gate */
#define SDT_SYSNULL2     8  /* system null again */
#define SDT_SYS386TSS    9  /* system 386 TSS available */
#define SDT_SYSNULL3    10  /* system null again */
#define SDT_SYS386BSY   11  /* system 386 TSS busy */
#define SDT_SYS386CGT   12  /* system 386 call gate */
#define SDT_SYSNULL4    13  /* system null again */
#define SDT_SYS386IGT   14  /* system 386 interrupt gate */
#define SDT_SYS386TGT   15  /* system 386 trap gate */

/* memory segment types */
#define SDT_MEMRO   16  /* memory read only */
#define SDT_MEMROA  17  /* memory read only accessed */
#define SDT_MEMRW   18  /* memory read write */
#define SDT_MEMRWA  19  /* memory read write accessed */
#define SDT_MEMROD  20  /* memory read only expand dwn limit */
#define SDT_MEMRODA 21  /* memory read only expand dwn limit accessed */
#define SDT_MEMRWD  22  /* memory read write expand dwn limit */
#define SDT_MEMRWDA 23  /* memory read write expand dwn limit accessed */
#define SDT_MEME    24  /* memory execute only */
#define SDT_MEMEA   25  /* memory execute only accessed */
#define SDT_MEMER   26  /* memory execute read */
#define SDT_MEMERA  27  /* memory execute read accessed */
#define SDT_MEMEC   28  /* memory execute only conforming */
#define SDT_MEMEAC  29  /* memory execute only accessed conforming */
#define SDT_MEMERC  30  /* memory execute read conforming */
#define SDT_MEMERAC 31  /* memory execute read accessed conforming */

#pragma pack(pop)

#endif
