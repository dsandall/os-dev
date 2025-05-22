#include "freestanding.h"
#include "rejigger_paging.h"

virt_addr_t kmalloc(size_t size);
void kfree(virt_addr_t addr);
