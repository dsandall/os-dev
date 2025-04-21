#ifndef TEXTBOX_H
#define TEXTBOX_H

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

void clear_Textbox(Textbox_t *box);
void VGA_printTest();
void printchar_vgatask(char c);

#endif
