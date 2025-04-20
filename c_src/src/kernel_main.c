#include "async.h"
#include "interrupts.h"
#include "pic_8259.h"
#include "printlib.h"
#include "ps2_keyboard.h"

extern void init_IDT(void);

extern run_result_t ps2_rx_task(void *s);
extern run_result_t vga_task(void *initial_state);

void kernel_main() {

  Textbox_t box = {.x_corner = 8,
                   .y_corner = 2,
                   .width = 60,
                   .height = 20,
                   .cursor = (position_t){8, 2}};

  VGA_printTest(&box);

  init_IDT();
  tracek("IDT initialized");
  do_PIC();
  tracek("PIC initialized");
  PIC_set_mask(0); // disable the timer
  tracek("masked the timer");
  init_PS2();

  // now ps2 is set up and you should be rxing keeb interrupts
  spawn_task(ps2_rx_task, NULL);
  spawn_task(vga_task, NULL);

  // Prepare to enter the matrix (by that I mean the async polling system)
  tracek("enabling interrupts with asm STI \n");
  __asm__("sti"); // enable interrupts // WARN:

  while (1) {
    run_tasks();
  }
}
