#include "async/async.h"
#include "interrupts.h"
#include "pic_8259.h"
#include "printer.h"
#include "ps2_keyboard.h"
#include <stdint.h>

// CREATE_IPC_CHANNEL_INSTANCE(vga_channel, ipc_channel_uint16,
// VGA_CHANNEL_SIZE);

// Allocating for the task (should only be spawned once):
// - 1 channel (from PS2_rx -> this task, the vga buffer)

run_result_t hw_int_task(void *initial_state) { return DEAD; }

run_result_t hw_int_task_init(void *initial_state) {

  init_IDT();
  debugk("IDT initialized\n");
  do_PIC();
  debugk("PIC initialized\n");
  PIC_set_mask(0); // disable the timer
  debugk("masked the timer\n");
  init_PS2();
  return PENDING;
}
