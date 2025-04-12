#include "printlib.h"
#include "ps2_keyboard.h"

void kernel_main() {

  Textbox_t box = {.x_corner = 8,
                   .y_corner = 2,
                   .width = 60,
                   .height = 20,
                   .cursor = (position_t){8, 2}};

  VGA_printTest(&box);

  while (1) {
    init_PS2();
  }
}
