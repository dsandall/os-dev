#ifndef MULTIBOOT_H
#define MULTIBOOT_H

// comes from multiboot header being placed first in the linker script.
#define MULTIBOOT_VADDR_OFFSET 0xFF000

#include "book.h"
#include "freestanding.h"

void fiftytwo_card_pickup();

#endif
