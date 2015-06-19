#ifndef PM_H
#define PM_H

#include "vm.h"

#define PM_NORDER   11
#define PM_NSEG     5

void        pm_init     (void);
void        pm_status   (void);
void        pm_add_seg  (vm_addr_t start, vm_addr_t end, vm_page_t* pages, uint32 npage);
vm_page_t*  pm_alloc    (uint8 order);
void        pm_release  (vm_page_t* page, uint8 order);

#endif
