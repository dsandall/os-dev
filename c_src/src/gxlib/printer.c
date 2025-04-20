#include "printer.h"

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
        ERR_LOOP();
        break;
      }
      break;
    }
  }

  va_end(va);
  return 0;
}
