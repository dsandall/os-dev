#include "vga_textbox.h"
#include "freestanding.h"
#include "vgalib.h"
#include <stdint.h>

#include "printer.h"

// Global State
Textbox_t *currentTextbox;

extern vga_color_t vga_fg_default;
extern vga_color_t vga_bg_default;
extern position_t VGA_cursor;

void set_Textbox(Textbox_t *box) {
  // user provides the struct that retains cursor and textbox details
  currentTextbox = box;
}

void clear_Textbox(void) {

  Textbox_t *box = currentTextbox;

  for (int x = box->x_corner; x < (box->width + box->x_corner); x++) {
    VGA_cursor.x = x;
    for (int y = box->y_corner; y < (box->height + box->y_corner); y++) {
      VGA_cursor.y = y;
      VGA_display_char(' ', VGA_DEFAULT, VGA_DEFAULT);
    }
  }

  box->cursor.x = box->x_corner;
  box->cursor.y = box->y_corner;
};

extern vga_char_t *VGA_ptr();

void scroll_Textbox(void) {

  Textbox_t *box = currentTextbox;

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

void clear_Line() {

  Textbox_t *box = currentTextbox;

  int y = box->height + box->y_corner - 1;
  for (int x = box->x_corner; x < (box->width + box->x_corner); x++) {
    VGA_cursor.y = y;
    VGA_cursor.x = x;
    VGA_display_char(' ', VGA_DEFAULT, VGA_DEFAULT);
  }
};

void print_char(char c) {
  Textbox_t *box = currentTextbox;
  position_t *tc = &currentTextbox->cursor;

  // Wrapping Logic
  // if (tc->y >= (box->height + box->y_corner)) {
  //  tc->y = box->y_corner;
  //  clear_Textbox();
  //}

  tc->y = box->y_corner + box->height - 1;

  switch (c) {
  case '\n':
    tc->x = box->x_corner;
    scroll_Textbox();
    tc->y = box->y_corner + box->height - 1;

    clear_Line();

    break;
  case '\r':
    tc->x = box->x_corner;
    break;
  default:

    VGA_cursor = *tc;
    VGA_display_char(c, VGA_DEFAULT, VGA_DEFAULT);

    tc->x++;
  }

  // Wrapping Logic
  // check bounds after updating text cursor position
  if (tc->x >= (box->width + box->x_corner)) {
    scroll_Textbox();

    clear_Line();
    tc->x = box->x_corner;
    tc->y = box->y_corner + box->height - 1;
  }
}

///////////////////////////////////
///////////////////////////////////
/// The entry point, window initialization:
///////////////////////////////////
///////////////////////////////////

void VGA_printTest(void);

void VGA_textbox_init(Textbox_t *box) {
  vga_bg_default = VGA_BLUE;
  vga_fg_default = VGA_BLUE;

  Textbox_t bar = {.x_corner = 0,
                   .y_corner = VGA_HEIGHT - 3,
                   .width = VGA_WIDTH,
                   .height = 2,
                   .cursor = (position_t){0, VGA_HEIGHT - 3}};

  set_Textbox(&bar);
  clear_Textbox();

  vga_bg_default = VGA_DARK_GREY;
  vga_fg_default = VGA_WHITE;

  set_Textbox(box);
  clear_Textbox();
  VGA_printTest();
};

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
  printk("%u\n", 4294967295);          // "4294967295"
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
