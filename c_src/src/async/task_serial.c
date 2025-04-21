#include "async/async.h"
#include "channel.h"
#include "freestanding.h"
#include "printer.h"
#include "serial.h"
#include <stdint.h>

CREATE_IPC_CHANNEL_INSTANCE(
    serial_ipc, uint16,
    UINT16_CHANNEL_SIZE); // allocating the ipc_channels (done in each task)

run_result_t hw_serial_task(void *initial_state) {
  serial_try_send();
  return PENDING;
}

run_result_t hw_serial_init(void *initial_state) {

  // init hardware, setup isr handler, masks
  SER_init(&serial_ipc);
  printk("oh yuh2:\n");

  printk("wassup");

  // TODO: set interrupt handler (currently done at compile time)

  // printk("ZYX\n"); // should print from serial

  return PENDING;
}

ISR_void isr_on_hw_serial(void) { serial_isr_handler(); }
