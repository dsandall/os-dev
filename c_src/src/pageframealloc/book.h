#ifndef BOOK_H
#define BOOK_H

#include "freestanding.h"
#include "rejigger_paging.h"
#define PAGE_SIZE 4096

typedef struct {
  uint64_t base;
  uint32_t size;
} phys_mem_region_t;

void testPageAllocator();

int makePhysPage(phys_mem_region_t available);

phys_addr MMU_pf_alloc(void);
phys_addr unsafe_MMU_pf_alloc(void);

bool MMU_pf_free(phys_addr pf);

void alloc();
void free();

#endif
