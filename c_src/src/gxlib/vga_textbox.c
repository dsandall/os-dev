#include "vga_textbox.h"
#include "freestanding.h"
#include "printer.h"
#include "vgalib.h"
#include <limits.h>

// Global State

// The other library just turns a cursor into a pointer to the vga buffer
extern position_t VGA_cursor;
extern vga_char_t *VGA_ptr();

void clear_Textbox(Textbox_t *box) {

  for (int x = box->x_corner; x < (box->width + box->x_corner); x++) {
    VGA_cursor.x = x;
    for (int y = box->y_corner; y < (box->height + box->y_corner); y++) {
      VGA_cursor.y = y;
      VGA_display_char(' ', box->fg, box->bg);
    }
  }

  box->cursor.x = box->x_corner;
  box->cursor.y = box->y_corner;
};

void scroll_Textbox(Textbox_t *box) {

  for (int y = box->y_corner; y < (box->height + box->y_corner - 1); y++) {
    for (int x = box->x_corner; x < (box->width + box->x_corner); x++) {

      VGA_cursor.y = y + 1;
      VGA_cursor.x = x;
      vga_char_t c = *VGA_ptr();

      VGA_cursor.y = y;
      VGA_cursor.x = x;
      VGA_display_char(c.character, c.fg_color, c.bg_color);
    }
  }
}

void clear_Line(Textbox_t *box) {

  int y = box->height + box->y_corner - 1;
  for (int x = box->x_corner; x < (box->width + box->x_corner); x++) {
    VGA_cursor.y = y;
    VGA_cursor.x = x;
    VGA_display_char(' ', box->fg, box->bg);
  }
};

void print_char_tobox_immediate(char c, Textbox_t *box) {
  position_t *tc = &box->cursor;

  // Wrapping Logic
  // if (tc->y >= (box->height + box->y_corner)) {
  //  tc->y = box->y_corner;
  //  clear_Textbox();
  //}

  tc->y = box->y_corner + box->height - 1;

  switch (c) {
  case '\n':
    tc->x = box->x_corner;
    scroll_Textbox(box);
    tc->y = box->y_corner + box->height - 1;

    clear_Line(box);

    break;
  case '\r':
    tc->x = box->x_corner;
    break;
  default:

    VGA_cursor = *tc;
    VGA_display_char(c, box->fg, box->bg);

    tc->x++;
  }

  // Wrapping Logic
  // check bounds after updating text cursor position
  if (tc->x >= (box->width + box->x_corner)) {
    scroll_Textbox(box);

    clear_Line(box);
    tc->x = box->x_corner;
    tc->y = box->y_corner + box->height - 1;
  }
}

///////////////////////////////////
///////////////////////////////////
/// The entry point, window initialization:
///////////////////////////////////
///////////////////////////////////

void VGA_printTest() {
  printk("%c\n", 'a');                 // should be "a"
  printk("%c\n", 'Q');                 // should be "Q"
  printk("%c\n", 256 + '9');           // Should be "9"
  printk("%s\n", "test string");       // "test string"
  printk("foo%sbar\n", "blah");        // "fooblahbar"
  printk("foo%%sbar\n");               // "foo%bar"
  printk("%d\n", INT_MIN);             // "-2147483648"
  printk("%d\n", INT_MAX);             // "2147483647"
  printk("%u\n", 0);                   // "0"
  printk("%x\n", 0xDEADbeef);          // "deadbeef"
  printk("%p\n", (void *)UINTPTR_MAX); // "0xFFFFFFFFFFFFFFFF"
  printk("%hd\n", 0x8000);             // "-32768"
  printk("%hd\n", 0x7FFF);             // "32767"
  printk("%hu\n", 0xFFFF);             // "65535" //TODO: FIX
  printk("%ld\n", LONG_MIN);           // "-9223372036854775808"
  printk("%ld\n", LONG_MAX);           // "9223372036854775807"
  printk("%lu\n", ULONG_MAX);          // "18446744073709551615"
  printk("%qd\n", LONG_MIN);           // "-9223372036854775808"
  printk("%qd\n", LONG_MAX);           // "9223372036854775807"
  printk("%qu\n", ULONG_MAX);          // "18446744073709551615"
}
