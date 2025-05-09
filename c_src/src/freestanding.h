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

typedef void ISR_void;

typedef struct position {
  int x;
  int y;
} position_t;

#define ERR_LOOP()                                                             \
  while (1) {                                                                  \
  }

//////////////
// Port IO
//////////////
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

//////////////
// Interrupts
//////////////
static inline bool are_interrupts_enabled(void) {
  unsigned long flags;
  __asm__ volatile("pushf\n\t" // Push EFLAGS onto the stack
                   "pop %0"
                   : "=g"(flags)
                   :
                   : "memory");
  return flags & (1 << 9); // IF is bit 9 of EFLAGS
}

#include "printer.h"

#define ASM_STI()                                                              \
  do {                                                                         \
    tracek("enabling interrupts\n");                                           \
    __asm__("sti");                                                            \
  } while (0)

#define ASM_CLI()                                                              \
  do {                                                                         \
    tracek("disabling interrupts\n");                                          \
    __asm__("cli");                                                            \
  } while (0)

static inline void RESUME(bool ints) {
  // enable if enabled
  if (ints) {
    ASM_STI();
  }
}
static inline bool PAUSE_INT(void) {
  bool ret = are_interrupts_enabled();
  // disable interrupts
  ASM_CLI();
  return ret;
}
#endif // FREESTANDING_H
