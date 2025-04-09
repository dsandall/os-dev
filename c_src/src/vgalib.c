#include "vgalib.h"

#define VGA_BASE_ADDR 0xb8000

// The VGA console is memory-mapped at address 0xb8000. Each character is stored
// as one 16 bit value in row-major order. The entire screen is 80x25.

// vga_base + y * VGA_WIDTH + x;
typedef struct {
  uint8_t character : 8; // 8 bits for the character
  uint8_t fg_color : 4;  // 4 bits for foreground color (0-15)
  uint8_t bg_color : 4;  // 4 bits for background color (0-15)
} __attribute__((packed)) vga_char_t;

static int vga_x;
static int vga_y;
static uint8_t vga_bg_default = VGA_DARK_GREY;
static uint8_t vga_fg_default = VGA_BRIGHT_PURPLE;

void VGA_setpos(int x, int y) {
  vga_x = x;
  vga_y = y;
  // TODO: add errors
}

vga_char_t *VGA_ptr() {
  // where 0,0 is top right
  int row = vga_y % (VGA_HEIGHT);
  int col = vga_x % (VGA_WIDTH);

  int offset = row * VGA_WIDTH + col;
  return (vga_char_t *)(VGA_BASE_ADDR + (size_t)offset * sizeof(vga_char_t));
}

void VGA_display_char(char c, uint8_t fg, uint8_t bg) {
  // use given, or default to existing
  vga_char_t vga_code = {c, fg ? fg : vga_fg_default, bg ? bg : vga_bg_default};
  *VGA_ptr() = vga_code;
};

// TODO: complete line (and block) wrapping logic
void Text_write_in(char c) {
  if (c == '\n') {
    VGA_setpos(0, vga_y + 1);
  } else if (c == '\r') {
    VGA_setpos(0, vga_y);
  } else {

    vga_x++;
    if (vga_x >= VGA_WIDTH) {
      vga_x = 0;
      vga_y++;
      if (vga_x >= VGA_HEIGHT) {
        vga_y = 0;
      }
    }

    // VGA_setpos(x, y);
    VGA_display_char(c, 0, 0);
  }
}

void VGA_clear(void) {
  for (int x = 0; x < VGA_WIDTH; x++) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
      VGA_setpos(x, y);
      VGA_display_char('.', VGA_PURPLE, VGA_PURPLE);
    }
  }
};

// TODO:
void VGA_display_str(const char *str) {};

__attribute__((format(printf, 1, 2))) int printk(const char *fmt, ...) {
  // %% %d %u %x %c %p %h[dux] %l[dux] %q[dux] %s
  // TODO:
};

// extern void print_char(char);
// extern void print_str(const char *);
// extern void print_uchar(unsigned char);
// extern void print_short(short);
// extern void print_long_hex(long);

void VGA_selftest(void) {
  // TODO:
  //  printk("%c", 'a');                 // should be "a"
  //  printk("%c", 'Q');                 // should be "Q"
  //  printk("%c", 256 + '9');           // Should be "9"
  //  printk("%s", "test string");       // "test string"
  //  printk("foo%sbar", "blah");        // "fooblahbar"
  //  printk("foo%%sbar");               // "foo%bar"
  //  printk("%d", INT_MIN);             // "-2147483648"
  //  printk("%d", INT_MAX);             // "2147483647"
  //  printk("%u", 0);                   // "0"
  //  printk("%u", UINT_MAX);            // "4294967295"
  //  printk("%x", 0xDEADbeef);          // "deadbeef"
  //  printk("%p", (void *)UINTPTR_MAX); // "0xFFFFFFFFFFFFFFFF"
  //  printk("%hd", 0x8000);             // "-32768"
  //  printk("%hd", 0x7FFF);             // "32767"
  //  printk("%hu", 0xFFFF);             // "65535"
  //  printk("%ld", LONG_MIN);           // "-9223372036854775808"
  //  printk("%ld", LONG_MAX);           // "9223372036854775807"
  //  printk("%lu", ULONG_MAX);          // "18446744073709551615"
  //  printk("%qd", LONG_MIN);           // "-9223372036854775808"
  //  printk("%qd", LONG_MAX);           // "9223372036854775807"
  //  printk("%qu", ULONG_MAX);          // "18446744073709551615"
}
