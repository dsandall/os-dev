#ifndef BOOK_H
#define BOOK_H

#include "freestanding.h"
#include "paging.h"
#include "regions.h"

void testPageAllocator();

int makePhysPage(phys_mem_region_t available);

phys_addr MMU_pf_alloc(void);
phys_addr unsafe_MMU_pf_alloc(void);

bool MMU_pf_free(phys_addr pf);

#endif
