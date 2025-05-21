#include "printer.h"
#include "freestanding.h"

static inline void print_str(void (*printc_fn)(char), const char *str) {
  while (*str != '\0') {
    // WARN: write char by char (hopefully it terminates, lol)
    printc_fn((char)(*str++));
  }
};

static inline void print_hex(void (*printc_fn)(char), uint64_t num) {
  const char hex_digits[] = "0123456789ABCDEF";
  int started = 0;

  // Print "0x" prefix
  printc_fn('0');
  printc_fn('x');

  // Loop through each nibble from most significant to least
  for (int i = (sizeof(uint64_t) * 8) - 4; i >= 0; i -= 4) {
    char digit = (num >> i) & 0xF;
    if (digit || started || i == 0) {
      printc_fn(hex_digits[(uint8_t)digit]);
      started = 1;
    }
  }
}

static inline void print_unsigned(void (*printc_fn)(char), uint64_t num) {
  if (num >= 10) {
    print_unsigned(printc_fn, num / 10);
  }
  printc_fn('0' + (num % 10));
}

static inline void print_signed(void (*printc_fn)(char), uint64_t num_abs,
                                bool is_neg) {
  if (is_neg) {
    printc_fn('-');
    print_unsigned(printc_fn, num_abs);
  } else {
    print_unsigned(printc_fn, num_abs);
  }
}

__attribute__((format(printf, 2, 0))) int
vprintk_agnostic(void (*printc_fn)(char), const char *fmt, va_list va) {

  char c;
  uint64_t n;
  bool is_neg;

  while ((c = *fmt++)) {
    switch (c) {
    default:
      printc_fn(c);
      break;
    case '\0':
      return 1;
    case '%':
      switch (c = *fmt++) {
      case '%':
        printc_fn('%');
        break;
      case 'p':
        print_hex(printc_fn, va_arg(va, size_t));
        break;
      case 's':
        print_str(printc_fn, va_arg(va, char *));
        break;
      case 'c':
        printc_fn((char)va_arg(va, int));
        break;
      case 'l':
      case 'q':
        n = va_arg(va, uint64_t);
        is_neg = (((int64_t)n) < 0);
        c = *fmt++;
        goto prant;
      case 'h':
        n = va_arg(va, uint32_t);
        is_neg = (((int16_t)n) < 0);
        c = *fmt++;
        goto prant;
      case 'd':
        n = va_arg(va, uint32_t);
        is_neg = (((int32_t)n) < 0);
        goto prant;
      case 'x':
      case 'u':
        n = va_arg(va, uint32_t);
        is_neg = (((int32_t)n) < 0);
        goto prant;

      prant:
        switch (c) {
        case 'd':
          print_signed(printc_fn, n, is_neg);
          break;
        case 'u':
          print_unsigned(printc_fn, n);
          break;
        case 'x':
        default:
          print_hex(printc_fn, n);
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

  return 0;
}

// sets the print fn used by any future print calls

static printfunction pc_fn_glbl = NULL;
void setPrinter(printfunction fn) { pc_fn_glbl = fn; };
static inline void printc_fn_safe(char c) {
  if (pc_fn_glbl == NULL) {
    ERR_LOOP();
  } else {
    pc_fn_glbl(c);
  }
};

__attribute__((format(printf, 1, 2))) int printk(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  int ret = vprintk_agnostic(printc_fn_safe, fmt, va);
  va_end(va);
  return ret;
}

extern void printchar_vgatask(char c);
__attribute__((format(printf, 1, 2))) int tracek_helper(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  int ret = vprintk_agnostic(printchar_vgatask, fmt, va);
  va_end(va);
  return ret;
}
