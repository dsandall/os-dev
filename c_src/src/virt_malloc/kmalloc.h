
#ifndef KMALLOC_H
#define KMALLOC_H

#include "freestanding.h"
#include "paging.h"

virt_addr_t kmalloc(size_t size);
void kfree(virt_addr_t addr);

#endif // KMALLOC_H
