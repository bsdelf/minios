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

static pgdir_t* _pgdir = NULL;

vm_object_t kernel_object;
vm_map_t kernel_map;

extern void LoadPageDirectory(uint32 dir);
extern void EnablePaging();

static void OnPageFault(IsrRegs regs);

static pte_t* kva2pte(uint32 va)
{
    uint32 ipage = va >> VM_PAGE_SHIFT;
    return &_pgdir->pte[ipage];
}

void vm_init(void)
{
    // retrieve page directory ptr
    _pgdir = (pgdir_t*)(env_bootinfo()->dir_va);
    screen_printf("dir: 0x%x\n", env_bootinfo()->video_va);

    // activate kernel stack guard
    {
        vm_addr_t guard = env_bootinfo()->stack_va;
        kva2pte(guard)->present = 0;
    }

    // note the video_pa is usually at 3.5GB or 3.75GB,
    // therefore it won't conusme any available physical memory we've detected.
    vm_addr_t pa_end = env_bootinfo()->dir_pa + 0x1000 * 1025;
    vm_addr_t va_end = env_bootinfo()->video_va + env_bootinfo()->video_size;

    // init vm_pages
    {
        // find out the segment with max available memory
        uint64 maxbeg = 0;
        uint64 maxlen = 0;
        for (int i = 0; i < env_bootinfo()->nhmem; ++i) {
            const smap_entry_t* entry = env_bootinfo()->ehmem+i;
            uint64 base = (((uint64)entry->baseh) << 32) + entry->basel;
            uint64 length = (((uint64)entry->lengthh) << 32) + entry->lengthl;

            if (entry->type == 1) {
                if (length > maxlen) {
                    maxbeg = base;
                    maxlen = length;
                }
            }
        }

        // TODO: 
        // 1. mask video_pa~(video_pa+video_size) from entry?
        // 2. align maxbeg and trunc maxlen?

        if (maxbeg != env_bootinfo()->kern_pa) {
            panic("maxbeg != kern_pa, though I know this restriction is stupid...");
        }
        if (pa_end < maxbeg || pa_end > maxbeg+maxlen) {
            panic("out of physical memory! this shouldn't happen...");
        }

        // use fix point iteration to calculate npage, maybe unnecessary
        uint64 maxused = env_bootinfo()->dir_pa - env_bootinfo()->kern_pa + 0x1000 * 1025;
        uint32 npage = (maxlen - maxused) / (VM_PAGE_SIZE + sizeof(vm_page_t));
        for (;;) {
            uint64 avail = maxlen - ALIGN_4K(sizeof(vm_page_t) * npage);
            uint32 npage2 = (avail >> VM_PAGE_SHIFT);
            if (npage <= npage2) {
                break;
            }
            --npage;
        }

        // permanently map a piece of memory for vm_pages
        uint32 sz = ALIGN_4K(sizeof(vm_page_t) * npage);
        for (vm_addr_t off = 0; off < sz; off += VM_PAGE_SIZE) {
            vm_mapva(va_end+off, pa_end+off, false, true);
        }
        vm_page_t* pages = (vm_page_t*)va_end;
        pa_end += sz;
        va_end += sz;

        // init vm_pages
        bzero(pages, sz);
        for (uint32 i = 0; i < npage; ++i) {
            pages[i].pa = pa_end + i * VM_PAGE_SIZE;
            pages[i].order = PM_NORDER;
        }

        // hand over vm_pages to pm
        pm_init();
        pm_add_seg(pa_end, pa_end+npage*VM_PAGE_SIZE, pages, npage);

        screen_printf("npage: %d avail: 0x%x\n", npage, npage*VM_PAGE_SIZE);
        screen_printf("end of vm_pages: %x end of maxseg: %x\n", pa_end+npage*VM_PAGE_SIZE, maxbeg+maxlen);
        pm_status();
    }

    /* init kernel heap */
    /*
    const uint32 heapsz = 2*1024*1024;
    {
        uint32 va = endofPm;
        uint32 pa = endofPm + poff;
        for (uint32 i = 0; i < heapsz; i += VM_PAGE_SIZE) {
            vm_page_t* page = pm_alloc(0);
            vm_mapva(va+i, page->pa, false, true);
        }
        HeapInit(va, heapsz);
    }
    */

    SetIsrHandler(IDT_PF, &OnPageFault);
    //LoadPageDirectory((uint32)&_pgdir->pde);
    //EnablePaging();
}

vm_addr_t vm_sbrk(uint32 subsym, vm_addr_t end)
{
    return 0;
}

void vm_mapva(uint32 va, uint32 pa, bool us, bool rw)
{
    pte_t* pte = kva2pte(va);
    pte->frame = pa >> VM_PAGE_SHIFT;
    pte->present = 1;
    pte->us = us ? 1 : 0;
    pte->rw = rw ? 1 : 0;
}

pgdir_t* ClonePageDirectory()
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
