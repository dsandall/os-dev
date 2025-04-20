#include "freestanding.h"
#include <stdint.h>

extern void print_char(char c);
// can be sourced from vga or serial driver

__attribute__((format(printf, 1, 2))) int printk(const char *fmt, ...);
