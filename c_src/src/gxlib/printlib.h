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

void VGA_printTest(Textbox_t *box);
