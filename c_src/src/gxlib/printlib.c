#include "printlib.h"
#include "vgalib.h"
#include <stdint.h>

Textbox_t *currentTextbox;

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

void scroll_Textbox(void) {

  Textbox_t *box = currentTextbox;

  for (int y = box->y_corner; y < (box->height + box->y_corner - 1); y++) {
    for (int x = box->x_corner; x < (box->width + box->x_corner); x++) {

      VGA_cursor.y = y + 1;
      VGA_cursor.x = x;
      vga_char_t c = VGA_get_char();

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

    tc->x = box->x_corner;
    tc->y = box->y_corner + box->height - 1;
  }
}

/*
void print_char_clearing(char c) {
  Textbox_t *box = currentTextbox;
  position_t *tc = &currentTextbox->cursor;

  // Wrapping Logic
  if (tc->y >= (box->height + box->y_corner)) {
    tc->y = box->y_corner;
    clear_Textbox();
  }

  switch (c) {
  case '\n':
    tc->x = box->x_corner;
    tc->y++;
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
    tc->x = box->x_corner;
    tc->y++;
  }
}
*/

void print_str(const char *str) {
  while (*str != '\0') {
    // WARN: write char by char (hopefully it terminates, lol)
    print_char((char)(*str++));
  }
};

void print_hex(uint64_t num) {
  const char hex_digits[] = "0123456789ABCDEF";
  int started = 0;

  // Print "0x" prefix
  print_char('0');
  print_char('x');

  // Loop through each nibble from most significant to least
  for (int i = (sizeof(uint64_t) * 8) - 4; i >= 0; i -= 4) {
    char digit = (num >> i) & 0xF;
    if (digit || started || i == 0) {
      print_char(hex_digits[(uint8_t)digit]);
      started = 1;
    }
  }
}

void print_unsigned(uint64_t num) {
  if (num >= 10) {
    print_unsigned(num / 10);
  }
  print_char('0' + (num % 10));
}

void print_signed(uint64_t num_abs, bool is_neg) {
  if (is_neg) {
    print_char('-');
    print_unsigned(num_abs);
  } else {
    print_unsigned(num_abs);
  }
}

__attribute__((format(printf, 1, 2))) int printk(const char *fmt, ...) {

  // %% %d %u %x %c %p %h[dux] %l[dux] %q[dux] %s
  va_list va;
  va_start(va, fmt);

  char c;
  uint64_t n;
  bool is_neg;

  while ((c = *fmt++)) {
    switch (c) {
    default:
      print_char(c);
      break;
    case '\0':
      va_end(va);
      return 1;
    case '%':
      // prepare to get next arg
      switch (c = *fmt++) {

      // we assume two % is a breakout for %
      case '%':
        print_char('%');
        break;

      // pointer printed as hex
      case 'p':
        print_hex(va_arg(va, size_t));
        break;

      // string
      case 's':
        print_str(va_arg(va, char *));
        break;

      // Char
      case 'c':
        print_char((char)va_arg(va, int));
        // char is promoted to int when used as variadic arg, must be cast back
        break;

        ///
        ///// Ints
        ///

        // Longs
      case 'l':
      case 'q':
        n = va_arg(va, uint64_t);
        is_neg = (((int64_t)n) < 0) ? true : false;
        c = *fmt++;
        goto prant;

      // Shorts
      case 'h':
        n = va_arg(va, uint32_t);
        // short is promoted to int when used as variadic arg
        is_neg = (((int16_t)n) < 0) ? true : false;
        c = *fmt++;
        goto prant;

      // "Ints"
      case 'd':
        n = va_arg(va, uint32_t);
        is_neg = (((int32_t)n) < 0) ? true : false;
        goto prant;
      case 'x':
      case 'u':
        n = va_arg(va, uint32_t);
        is_neg = (((int32_t)n) < 0) ? true : false;
        goto prant;

      prant:
        switch (c) {
        case 'd':
          print_signed(n, is_neg);
          break;
        case 'u':
          print_unsigned(n);
          break;
        case 'x':
        default:
          print_hex(n);
          break;
        }
        break;

      default:
        while (1) {
          // WARN: no way to print this... but i can make a loop here!
        }
        break;
      }
      break;
    }
  }

  va_end(va);
  return 0;
}

void Text_Taskbar(void) {
  vga_bg_default = VGA_BLUE;
  vga_fg_default = VGA_BLUE;

  Textbox_t bar = {.x_corner = 0,
                   .y_corner = VGA_HEIGHT - 3,
                   .width = VGA_WIDTH,
                   .height = 2,
                   .cursor = (position_t){0, VGA_HEIGHT - 3}};

  set_Textbox(&bar);
  clear_Textbox();
}

void VGA_printTest(Textbox_t *box) {

  Text_Taskbar();

  vga_bg_default = VGA_DARK_GREY;
  vga_fg_default = VGA_WHITE;

  set_Textbox(box);
  clear_Textbox();

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
  //
}
