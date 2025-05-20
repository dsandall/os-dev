#ifndef BOOK_H
#define BOOK_H

#include "freestanding.h"
#define PAGE_SIZE 4096

typedef struct {
  uint64_t base;
  uint32_t size;
} phys_mem_region_t;

void testPageAllocator();
void testPageAllocator_stresstest();

int makePhysPage(phys_mem_region_t available);

void *MMU_pf_alloc(void);
bool MMU_pf_free(void *pf);

void alloc();
void free();

#endif
