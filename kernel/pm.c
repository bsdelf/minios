#include "pm.h"
#include "panic.h"
#include "screen.h"

/* physical segments */
typedef struct {
    vm_addr_t start;
    vm_addr_t end;
    vm_page_t* pages;
} phys_seg_t;

static phys_seg_t _segs[PM_NSEG];
static uint8 _nseg;

/* free lists */

typedef struct {
    TAILQ_HEAD(, vm_page) pages;
    uint32 count;
} freelist_t;

static freelist_t _freelists[PM_NORDER];

/* internal utilities */
static void pm_split(vm_page_t* page, uint8 oind, uint8 order);

void pm_init(void)
{
    _nseg = 0;
    bzero(_segs, sizeof(_segs));
    for (uint8 i = 0; i < PM_NORDER; ++i) {
        freelist_t* fl = &_freelists[i];
        TAILQ_INIT(&fl->pages);
        fl->count = 0;
    }
}

void pm_status()
{
    for (int i = PM_NORDER-1; i >= 0; --i) {
        freelist_t* fl = &_freelists[i];
        screen_printf("%2d (%4d K): %u\n", i, (1<<i)*4, fl->count);
    }
}

void pm_add_seg(vm_addr_t start, vm_addr_t end, vm_page_t* pages, uint32 npage)
{
    phys_seg_t* seg = &_segs[_nseg];
    seg->start = start;
    seg->end = end;
    seg->pages = pages;
    for (uint32 i = 0; i < npage; ++i) {
        pages[i].seg = _nseg;
        pm_release(pages+i, 0);
    }
    _nseg++;
}

vm_page_t* pm_alloc(uint8 order)
{
    for (uint8 oind = order; oind < PM_NORDER; ++oind) {
        freelist_t* fl = &_freelists[oind];
        vm_page_t* head = TAILQ_FIRST(&fl->pages);
        if (head != NULL) {
            TAILQ_REMOVE(&fl->pages, head, __pgqnode__);
            fl->count--;
            head->order = PM_NORDER;
            pm_split(head, oind, order);
            return head;
        }
    }
    return NULL;
}

static void pm_split(vm_page_t* page, uint8 oind, uint8 order)
{
    for (; oind > order; ) {
        oind--;
        vm_page_t* buddy = &page[1 << oind];
        buddy->order = oind;
        freelist_t* fl = &_freelists[oind];
        TAILQ_INSERT_HEAD(&fl->pages, buddy, __pgqnode__);
        fl->count++;
    }
}

void pm_release(vm_page_t* page, uint8 order)
{
    const phys_seg_t* seg = &_segs[page->seg];
    vm_addr_t pa = page->pa;

    while (order < PM_NORDER-1) {
        vm_addr_t pa_buddy = pa ^ (1 << (VM_PAGE_SHIFT + order));
        if (pa_buddy < seg->start || pa_buddy >= seg->end)
            break;

        vm_page_t* buddy = &seg->pages[atop(pa_buddy - seg->start)];
        if (buddy->order != order)
            break;
        freelist_t* fl = &_freelists[buddy->order];
        TAILQ_REMOVE(&fl->pages, buddy, __pgqnode__);
        fl->count--;
        buddy->order = PM_NORDER;

        order++;
        pa &= ~((1 << (VM_PAGE_SHIFT + order)) - 1);
        page = &seg->pages[atop(pa - seg->start)];
    }

    page->order = order;
    freelist_t* fl = &_freelists[order];
    TAILQ_INSERT_TAIL(&fl->pages, page, __pgqnode__);
    fl->count++;
}
