#ifndef FREESTANDING_H
#define FREESTANDING_H

// https://wiki.osdev.org/C_Library#Freestanding_and_Hosted
// #include <float.h>
// #include <iso646.h>
#include <limits.h>
// #include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
//   #include <stdnoreturn.h>
//

typedef struct position {
  int x;
  int y;
} position_t;

static void io_wait(void);
static __inline unsigned char inb(unsigned short int __port) {
  unsigned char _v;
  __asm__ __volatile__("inb %w1,%0" : "=a"(_v) : "Nd"(__port));
  return _v;
}
static __inline void outb(unsigned short int __port, unsigned char __value) {
  __asm__ __volatile__("outb %b0,%w1" : : "a"(__value), "Nd"(__port));
  // io_wait();
}

static inline void io_wait(void) { outb(0x80, 0); }

#endif // FREESTANDING_H
