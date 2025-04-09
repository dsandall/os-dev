#include "freestanding.h"
#include "vgalib.h"

void kernel_main() {
  while (1) {
    for (int x = 0; x < VGA_WIDTH; x++) {
      for (int y = 0; y < VGA_HEIGHT; y++) {
        VGA_setpos(x, y);
        Text_write_in((char)(x % 10 + 48));
      }
    }
    VGA_clear();
  }
}
