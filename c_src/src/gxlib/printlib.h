#ifndef PRINTLIB_H
#define PRINTLIB_H

#include "freestanding.h"

typedef struct {
  int x_corner;
  int y_corner;
  int height;
  int width;
  position_t cursor;
} Textbox_t;

void Text_write_in(char c);

void set_Textbox(Textbox_t *box);

__attribute__((format(printf, 1, 2))) int printk(const char *fmt, ...);

// Define this macro to enable or disable debug printing
#define ENABLE_DEBUG 1
#if ENABLE_DEBUG
#define debugk(fmt, ...) printk("DBG - " fmt, ##__VA_ARGS__)

#else
#define debugk(...) // Do nothing
#endif              // ENABLE_DEBUG

#define ENABLE_TRACE 0
#if ENABLE_TRACE
#define tracek(fmt, ...) printk("TRC - " fmt, ##__VA_ARGS__)

#else
#define tracek(...) // Do nothing
#endif              // ENABLE_TRACE

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

void VGA_printTest(Textbox_t *box);

#endif
