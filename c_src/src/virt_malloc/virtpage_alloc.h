
#ifndef VIRTPAGE_ALLOC_H
#define VIRTPAGE_ALLOC_H

#include "rejigger_paging.h"

bool is_in_kheap(virt_addr_t v);
bool has_been_demanded(virt_addr_t v);

virt_addr_t MMU_alloc_page(void);
bool MMU_free_page(virt_addr_t v);

ISR_void pageFault_handler(uint32_t error);

#endif // VIRTPAGE_ALLOC_H
