#ifndef PRINTLIB_H
#define PRINTLIB_H

#include "freestanding.h"
#include "vgalib.h"

typedef struct {
  int x_corner;
  int y_corner;
  int height;
  int width;
  position_t cursor;
  vga_color_t bg;
  vga_color_t fg;
} Textbox_t;

void VGA_textbox_init(Textbox_t *box);
void clear_Textbox(Textbox_t *box);
void VGA_printTest();
void printchar_defaultHandler(char c);

#endif
