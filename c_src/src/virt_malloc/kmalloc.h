
#ifndef KMALLOC_H
#define KMALLOC_H

#include "freestanding.h"
#include "paging.h"

void *kmalloc(size_t size);
void kfree(void *addr);

#endif // KMALLOC_H
