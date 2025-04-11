#include "printlib.h"

void kernel_main() {

  while (1) {
    // VGA_cursor.x = 0;
    // VGA_cursor.y = 0;
    // printk("1");

    // VGA_cursor.x = VGA_WIDTH - 1;
    // VGA_cursor.y = 0;
    // printk("2");

    // VGA_cursor.x = 0;
    // VGA_cursor.y = VGA_HEIGHT - 1;
    // printk("3");

    // VGA_cursor.x = VGA_WIDTH - 1;
    // VGA_cursor.y = VGA_HEIGHT - 1;
    // printk("4");

    // printk("... hi");
    // printk("\nhi hello\n haii");

    VGA_printTest();
  }
}
