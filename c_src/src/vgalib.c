#include "vgalib.h"

#define VGA_BASE_ADDR 0xb8000

// The VGA console is memory-mapped at address 0xb8000. Each character is stored
// as one 16 bit value in row-major order. The entire screen is 80x25.

// vga_base + y * VGA_WIDTH + x;

position_t VGA_cursor;
uint8_t vga_bg_default = VGA_DARK_GREY;
uint8_t vga_fg_default = VGA_BRIGHT_PURPLE;

vga_char_t *VGA_ptr() {
  // where 0,0 is top right
  int row = VGA_cursor.y % (VGA_HEIGHT);
  int col = VGA_cursor.x % (VGA_WIDTH);
  int offset = row * VGA_WIDTH + col;
  return (vga_char_t *)(VGA_BASE_ADDR + (size_t)offset * sizeof(vga_char_t));
}

void VGA_display_char(char c, uint8_t fg, uint8_t bg) {
  // use given, or default to existing
  vga_char_t vga_code = {c, (fg != VGA_DEFAULT) ? fg : vga_fg_default,
                         (bg != VGA_DEFAULT) ? bg : vga_bg_default};
  *VGA_ptr() = vga_code;
};

void VGA_clear(void) {
  for (int x = 0; x < VGA_WIDTH; x++) {
    VGA_cursor.x = x;
    for (int y = 0; y < VGA_HEIGHT; y++) {
      VGA_cursor.y = y;
      VGA_display_char('.', VGA_BLACK, VGA_BLACK);
    }
  }
};
