#include "interrupts.h"
#include "printlib.h"
#include "ps2_keyboard.h"

extern void init_IDT(void);
extern void do_PIC(void);
extern void IRQ_set_mask(uint8_t IRQline);
extern volatile uint32_t isr_flag;
extern void isr_common_handler(int isr_flag);
extern int async_main(void);

void kernel_main() {

  Textbox_t box = {.x_corner = 8,
                   .y_corner = 2,
                   .width = 60,
                   .height = 20,
                   .cursor = (position_t){8, 2}};

  VGA_printTest(&box);

  init_IDT();
  do_PIC();
  IRQ_set_mask(0); // disable the timer
  init_PS2();

  // now ps2 is set up and you should be rxing keeb interrupts

  async_main();
}
