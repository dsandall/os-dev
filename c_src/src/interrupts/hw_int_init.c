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

void hw_int_init() {
  init_IDT();
  tracek("IDT initialized\n");
  do_PIC();
  tracek("PIC initialized\n");
  PIC_set_mask(0); // disable the timer
  tracek("masked the timer\n");
  init_PS2();
}
