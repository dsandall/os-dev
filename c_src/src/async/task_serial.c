#include "async/async.h"
#include "channel.h"
#include "freestanding.h"
#include "printer.h"
#include "serial.h"
#include <stdint.h>

// Allocating for the task (should only be spawned once):
// - 1 channel (from PS2_rx -> this task, the vga buffer)
CREATE_IPC_CHANNEL_INSTANCE(serial_ipc, ipc_channel_uint8, UINT8_CHANNEL_SIZE);

run_result_t hw_serial_task(void *initial_state) {
  serial_try_send();
  return PENDING;
}

run_result_t hw_serial_init(void *initial_state) {

  // init hardware, setup isr handler, masks
  SER_init(&serial_ipc);

  // TODO: set interrupt handler (currently done at compile time)

  setPrinter(SER_printc);

  printk("ZYX\n");

  return PENDING;
}

ISR_void isr_on_hw_serial(void) { serial_isr_handler(); }
