#ifndef BOOK_H
#define BOOK_H

#include "freestanding.h"

typedef struct {
  uint64_t base;
  uint32_t size;
} phys_mem_region_t;

void initPageAllocator();
void makePage(phys_mem_region_t available);
void takePage(phys_mem_region_t not_available);

void alloc();
void free();

#endif
