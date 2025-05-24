#ifndef TESTER_H
#define TESTER_H

#include "freestanding.h"

#define PHYSICAL_ALLOCATOR_STRESSTEST
#define VIRT_ALLOCATOR_STRESSTEST
#define KMALLOC_STRESSTEST

void testPageAllocator();
void testVirtPageAlloc();
void testKmalloc();

#endif // TESTER_H
