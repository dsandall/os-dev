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

void VGA_printTest(Textbox_t *box);
