#include "vm.h"
#include "pm.h"
#include "env.h"
#include "screen.h"
#include "IDT.h"
#include "Heap.h"
#include "panic.h"
#include "asm.h"

#include <stand.h>
#include <Bitmap.h>

vm_object_t kernel_object;
vm_map_t kernel_map;

static void OnPageFault(IsrRegs regs);

//static bool _init = false;
static uint32 _kernPaddr = 0;
static uint32 _kernVaddr = 0;

static KernelDirectory* _dir = NULL;

extern void LoadPageDirectory(uint32 dir);
extern void EnablePaging();

static pte_t* kva2pte(uint32 va)
{
    // TODO: rmove hard code
    uint32 ipage = va >> VM_PAGE_SHIFT;
    uint32 itable = ipage / 1024;
    likely (itable >= 768) {
        // 3G~4G
        return &_dir->tables[itable-768+1].pte[ipage%1024];
    } else if (itable == 0) {
        // 0~4M
        return &_dir->tables[0].pte[ipage];
    }
    return NULL;
}

void vm_init(void)
{
    /* Virtual memory layout
     *
     * -------------------
     *  kernel         nK                           \
     * -------------------                           \
     *  guard          4K \                           |
     * ------------------- +--> kernel stack          |
     *  stack          nK /                           +--> loader knows
     * -------------------                            |
     *  dir            4K \                           | 
     * ------------------- +--> KernelDirectory      /
     *  tables         1M /                         /
     * ------------------- 
     *  vm_page        nK
     * -------------------
     *  video        w*hB
     * -------------------
     *  UMA            nM
     * -------------------
     *
     */

    uint64 maxbeg = 0;
    uint64 maxlen = 0;
    for (int i = 0; i < env_get()->nhmem; ++i) {
        const SmapEntry* entry = env_get()->ehmem+i;
        uint64 base = (((uint64)entry->baseh) << 32) + entry->basel;
        uint64 length = (((uint64)entry->lengthh) << 32) + entry->lengthl;

        if (entry->type == 1) {
            if (length > maxlen) {
                maxbeg = base;
                maxlen = length;
            }
        }
    }

    _kernPaddr = env_get()->kern_pa;
    _kernVaddr = env_get()->kern_va;
    const int64 poff =  (int64)_kernPaddr - _kernVaddr;

    /* init dir (loader already did it) */
    _dir = (KernelDirectory*)(env_get()->stack_va + env_get()->stack_size);
    uint32 endofDir = (uint32)_dir + sizeof(KernelDirectory);

    /* init physical memory management */
    /* TODO:
     * 1. don't assume kernel at the beginning of the maximum segment
     * 2. support multi-segment
     */
    pm_init();
    uint32 endofPm = 0;
    {
        uint32 va = endofDir;
        uint32 pa = va + poff;

        // use maximum segment only
        uint32 npage = 0;
        for (uint32 off = pa; off < maxbeg + maxlen; off += VM_PAGE_SIZE) {
            ++npage;
        }
        uint32 size = sizeof(vm_page_t) * npage;
        size = ALIGN_4K(size);
        for (vm_addr_t i = 0; i < size; i += VM_PAGE_SIZE) {
            vm_mapva(va+i, pa+i, false, true);
        }

        // init pages for the segment
        vm_page_t* pages = (vm_page_t*)va;
        bzero(pages, size);
        for (uint32 i = 0; i < npage; ++i) {
            pages[i].pa = pa + i * VM_PAGE_SIZE;
            pages[i].order = PM_NORDER;
        }

        // give it to pm
        pm_add_seg(pa, maxbeg+maxlen, pages, npage);
        endofPm = va + size;
    }

    /* init video memory */
    /* TODO:
     * We based on an assumption that
     * the physical address of the video memory(usually very high)
     * won't be conflict with the memory we've already used.
     * This might be a potential issue.
     */
    uint32 endofVideo = 0;
    {
        uint32 pa = env_get()->vram;
        uint32 va = ALIGN_4K(endofPm);
        uint32 size = ALIGN_4K(env_get()->xpixel * env_get()->ypixel);
        endofVideo = va + size;
        for (uint32 i = 0; i < size; i += VM_PAGE_SIZE) {
            vm_mapva(va+i, pa+i, false, true);
        }
        env_get()->vram = va;
        setvram();
    }

    screen_clear();
    pm_status();

    /* init kernel heap */
    const uint32 heapsz = 5*1024*1024;
    {
        uint32 va = endofVideo;
        uint32 pa = endofVideo + poff;
        for (uint32 i = 0; i < heapsz; i += VM_PAGE_SIZE) {
            vm_page_t* page = pm_alloc(0);
            vm_mapva(va+i, page->pa, false, true);
        }
        HeapInit(va, heapsz);
    }
    
    /* activate kernel stack guard */
    {
        uint32 guard = env_get()->stack_va;
        kva2pte(guard)->present = 0;
    }

    /*
    // code
    AllocFrameAt(1*1024*1024*20, VADDR2PTE(_dir, 0), false, true);
    _dir->pde[1*1024*1024*20/0x1000/1024].us = 1;

    // context+stack
    AllocFrameAt(1*1024*1024*20 + 0x1000, VADDR2PTE(_dir, 0x1000), false, true);
    _dir->pde[(1*1024*1024*20/+0x1000)/0x1000/1024].us = 1;

    AllocFrameAt(1*1024*1024*20 + 0x2000, VADDR2PTE(_dir, 0x2000), false, true);
    _dir->pde[(1*1024*1024*20+0x2000)/0x1000/1024].us = 1;

    // tss stack
    AllocFrameAt(1*1024*1024*21-0x1000, VADDR2PTE(_dir, 1*1024*1024*21-0x1000), false, true);
    _dir->pde[(1*1024*1024821-0x1000)/0x1000/1024].us = 1;
    */

    setvram();
    SetIsrHandler(IDT_PF, &OnPageFault);
    //LoadPageDirectory((uint32)&_dir->pde);
    //EnablePaging();
}

vm_addr_t vm_sbrk(uint32 subsym, vm_addr_t end)
{
}

void vm_mapva(uint32 va, uint32 pa, bool us, bool rw)
{
    uint32 iframe = pa >> VM_PAGE_SHIFT;

    pte_t* pte = kva2pte(va);
    pte->frame = iframe;
    pte->present = 1;
    pte->us = us ? 1 : 0;
    pte->rw = rw ? 1 : 0;
}

KernelDirectory* ClonePageDirectory()
{
    return 0;
}

void OnPageFault(const IsrRegs regs)
{
    uint32 faultAt = 0;
    __asm__ volatile("mov %%cr2, %0" : "=r" (faultAt));

    const pte_t* pte = (pte_t*)&regs.err_code;

    char msg[1024];
    sprintf(msg, "0x%x, p:%d, rw:%d, us:%d", faultAt, pte->present, pte->rw, pte->us);
    panic(msg);
}
