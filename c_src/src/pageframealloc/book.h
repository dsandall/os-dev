#ifndef BOOK_H
#define BOOK_H

#include "freestanding.h"
#define PAGE_SIZE 4096

typedef struct {
  uint64_t base;
  uint32_t size;
} phys_mem_region_t;

void initPageAllocator();
void testPageAllocator();

int makePage(phys_mem_region_t available);
void takePage(phys_mem_region_t not_available);

void alloc();
void free();

#endif
