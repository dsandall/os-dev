#include "interrupts.h"
#include "printlib.h"
#include "ps2_keyboard.h"

extern void init_IDT(void);
extern void do_PIC(void);

void kernel_main() {

  Textbox_t box = {.x_corner = 8,
                   .y_corner = 2,
                   .width = 60,
                   .height = 20,
                   .cursor = (position_t){8, 2}};

  VGA_printTest(&box);

  init_IDT();
  do_PIC();

  printk("nugget\n");

  __asm__("int $12"); // Breakpoint interrupt (doesnt seem to do anything)
  printk("nugget 12\n");
  __asm__("int $15"); // Breakpoint interrupt (doesnt seem to do anything)
  printk("nugget 15\n");
  __asm__("int $220"); // Breakpoint interrupt (doesnt seem to do anything)
  printk("nugget 18\n");
  __asm__("int $30"); // Breakpoint interrupt (doesnt seem to do anything)
  printk("nugget 33\n");
  __asm__("int $31"); // Breakpoint interrupt (doesnt seem to do anything)
  printk("nugget 40\n");
  __asm__("int $32"); // Breakpoint interrupt (doesnt seem to do anything)
  printk("nugget winner\n");

  init_PS2();
}
