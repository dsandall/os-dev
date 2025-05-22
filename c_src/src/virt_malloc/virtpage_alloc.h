
#ifndef VIRTPAGE_ALLOC_H
#define VIRTPAGE_ALLOC_H

#include "rejigger_paging.h"

extern virt_addr_t heap_pointer;

bool is_in_kheap(virt_addr_t v);

virt_addr_t MMU_alloc_page(void);
void MMU_free_page(virt_addr_t v);

void testVirtPageAlloc();
#endif // VIRTPAGE_ALLOC_H
