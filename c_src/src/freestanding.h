#ifndef FREESTANDING_H
#define FREESTANDING_H

// https://wiki.osdev.org/C_Library#Freestanding_and_Hosted
// #include <float.h>
// #include <iso646.h>
// #include <limits.h>
// #include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t strlen(const char *s);
void *memcpy(void *dest, const void *src, size_t n);

//  __asm__("int $0x20");
static void breakpoint() {};

#define BREAK_IF(condition)                                                    \
  if (condition) {                                                             \
    breakpoint();                                                              \
  }

#define ERR_LOOP()                                                             \
  while (1) {                                                                  \
  }

#define ASSERT(condition)                                                      \
  while (!(condition)) {                                                       \
    ERR_LOOP();                                                                \
  }

static __inline int div_round_up(int numerator, int denominator) {
  return (numerator + denominator - 1) / denominator;
}

//////////////
// Port IO
//////////////
static __inline unsigned char inb(unsigned short int __port) {
  unsigned char _v;
  asm volatile("inb %w1,%0" : "=a"(_v) : "Nd"(__port));
  return _v;
}
static __inline void outb(unsigned short int __port, unsigned char __value) {
  asm volatile("outb %b0,%w1" : : "a"(__value), "Nd"(__port));
  // io_wait();
}

static inline void io_wait(void) { outb(0x80, 0); }

//////////////
// Interrupts
//////////////

typedef void ISR_void;

static inline void RESUME(bool ints) {
  // enable if previously enabled
  if (ints) {
    asm("sti");
  }
}

static inline bool PAUSE_INT(void) {
  unsigned long flags;
  // stack nonsense required to read the reg
  asm volatile("pushf\n\tpop %0" : "=g"(flags)::"memory");

  // disable interrupts
  asm("cli");

  // return the previous state of the interrupts
  return flags & (1 << 9);
}

#endif // FREESTANDING_H
