#ifndef VM_H
#define VM_H

#include <types.h>
#include <queue.h>

#define SUBSYM_UMA      1

#define VM_PAGE_SHIFT   12
#define VM_PAGE_SIZE    0x1000
#define VM_PAGE_MASK    (VM_PAGE_SIZE-1)

#define trunc_page(x)       ((x) & ~VM_PAGE_MASK)
#define round_page(x)       (((x) + VM_PAGE_MASK) & ~VM_PAGE_MASK)
#define atop(x)             ((x) >> VM_PAGE_SHIFT)
#define ptoa(x)             ((x) << VM_PAGE_SHIFT)

/*
 * Format of a linear(virtual) address
 * |31       22|21      12|11       0|
 * |DIR        |PAGE      |OFFSET    |
 * |10bits     |10bits    |12bits    |
 * |1024       |1024      |4K        |
 */

typedef uint32 vm_addr_t;

typedef int64 vm_offset_t;

#pragma pack(push, 1)

typedef struct {
    uint32 present      : 1;    // page present in memory
    uint32 rw           : 1;    // read-only(0), read-write(1)
    uint32 us           : 1;    // user/supervisor level(1/0)
    uint32 _1           : 2;    // reserved
    uint32 accessed     : 1;    // accessed
    uint32 dirty        : 1;    // written
    uint32 _2           : 2;    // reserved
    uint32 avail        : 3;    // available
    uint32 frame        : 20;   // frame address (shifted right 12 bits)
} pte_t; // 4B

typedef struct {
    uint32 present      : 1;    // page present in memory
    uint32 rw           : 1;    // read-only(0), read-write(1)
    uint32 us           : 1;    // user/supervisor level(1/0)
    uint32 _1           : 2;    // reserved
    uint32 accessed     : 1;    // accessed
    uint32 _2           : 1;    // reserved
    uint32 size         : 1;    // 4K(0), 4M(1)
    uint32 _3           : 1;    // reserved
    uint32 avail        : 3;    // available
    uint32 frame        : 20;   // frame address (shifted right 12 bits)
} pde_t; // 4B

typedef struct {
    pde_t pdes[1024];
    pte_t ptes[1024*1024];
} pgdir_t; // 4K+4M

#pragma pack(pop)

typedef struct vm_page {
    vm_addr_t pa;
    uint8 seg;

    uint8 order;

    uint32 ref;
    bool dirty;

    TAILQ_ENTRY(vm_page) __pgqnode__;
    TAILQ_ENTRY(vm_page) __objnode__;
} vm_page_t;

typedef struct vm_object {
    TAILQ_HEAD(, vm_page) pages;

    vm_page_t resident;
    vm_page_t cache;

    void* pghandle;
    union {
        struct {
        } vnp;
        struct {
        } swp;
    } pager;

    TAILQ_ENTRY(vm_object) __objnode__;
} vm_object_t;

typedef struct {
} vm_map_t;

typedef struct {
} vmspace_t;

extern vm_object_t kernel_object;
extern vm_map_t kernel_map;

void vm_init();
void vm_mapva(uint32 va, uint32 pa, bool us, bool rw);
vm_addr_t vm_sbrk(uint32 subsym, vm_addr_t end);

pte_t* GetPageTableEntry(pgdir_t* dir, uint32 addr);
pgdir_t* ClonePageDirectory();
void SwitchPageDirectory();

#endif
