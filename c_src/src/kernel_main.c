#include "async.h"
#include "interrupts.h"
#include "pic_8259.h"
#include "ps2_keyboard.h"
#include "vga_textbox.h"

extern void init_IDT(void);

extern run_result_t ps2_rx_task(void *s);
extern run_result_t vga_task(void *initial_state);

extern run_result_t vga_task_init(void *initial_state);
void kernel_main() {

  // VGA_textbox_init(&box);

  init_IDT();
  debugk("IDT initialized");
  do_PIC();
  debugk("PIC initialized");
  PIC_set_mask(0); // disable the timer
  debugk("masked the timer");
  init_PS2();

  // now ps2 is set up and you should be rxing keeb interrupts
  spawn_task(ps2_rx_task, NULL, NULL);
  spawn_task(vga_task, NULL, vga_task_init);

  // Prepare to enter the matrix (by that I mean the async polling system)
  ASM_STI(); // enable interrupts // WARN:

  while (1) {
    run_tasks();
  }
}
