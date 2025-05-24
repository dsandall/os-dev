#ifndef TESTER_H
#define TESTER_H

#include "freestanding.h"

#define HASH(a, b, c) ((a % 17) * b) + c

bool generic_page_tester(uint64_t *static_array, uint64_t static_len,
                         uint64_t (*generic_alloc)(void),
                         bool (*generic_free)(uint64_t));

void testPageAllocator();
void testVirtPageAlloc();
void testKmalloc();

#endif // TESTER_H
