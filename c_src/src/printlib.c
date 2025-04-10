#include "printlib.h"
#include "vgalib.h"

Textbox_t *currentTextbox;

void set_Textbox(Textbox_t *box) {
  // user provides the struct that retains cursor and textbox details
  currentTextbox = box;
}

void print_char(char c) {
  Textbox_t *box = currentTextbox;
  position_t *tc = &currentTextbox->cursor;

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
    VGA_display_char(c, 0, 0);

    tc->x++;
  }

  // check bounds after updating text cursor position
  if (tc->x >= (box->width + box->x_corner)) {
    tc->x = 0;
    tc->y++;
  }
  if (tc->y >= (box->height + box->y_corner)) {
    tc->y = 0;
  }
}

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
      print_char(hex_digits[digit]);
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

void print_signed(uint64_t num, bool is_neg) {
  if (is_neg) {
    print_char('-');
    print_unsigned(-num);
  } else {
    print_unsigned(num);
  }
}

// void print_number(uint64_t num, enum format fmt) {
//   if (fmt == Hex) {
//     print_hex(num); // Print in hexadecimal format
//   } else if (fmt == Unsigned) {
//     print_unsigned(num); // Print in unsigned decimal format
//   } else if (fmt == Signed) {
//     print_signed(num); // Print in signed decimal format
//   }
// }

// TODO:
//__attribute__((format(printf, 1, 2)))
int printk(const char *fmt, ...) {

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
        n = va_arg(va, int64_t);
        is_neg = (((int64_t)n) < 0) ? true : false;
        c = *fmt++;
        goto prant;

      // Shorts
      case 'h':
        n = (va_arg(va, int32_t));
        // short is promoted to int when used as variadic arg
        is_neg = (((int16_t)n) < 0) ? true : false;
        c = *fmt++;
        goto prant;

      // "Ints"
      case 'd':
      case 'u':
      case 'x':
        n = va_arg(va, int32_t);
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

void VGA_printTest(void) {
  VGA_cursor.x = 0;
  VGA_cursor.y = 0;

  printk("%c\n", 'a');                 // should be "a"
  printk("%c\n", 'Q');                 // should be "Q"
  printk("%c\n", 256 + '9');           // Should be "9"
  printk("%s\n", "test string");       // "test string"
  printk("foo%sbar\n", "blah");        // "fooblahbar"
  printk("foo%%sbar\n");               // "foo%bar"
  printk("%d\n", INT_MIN);             // "-2147483648"
  printk("%d\n", INT_MAX);             // "2147483647"
  printk("%u\n", 0);                   // "0"
  printk("%u\n", UINT_MAX);            // "4294967295"
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

  VGA_clear();
}
