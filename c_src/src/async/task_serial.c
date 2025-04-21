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
  serial_try_send(); // WARN : should only be needed if it gets lost... poll
                     // infrequently
  return PENDING;
}

void printchar_serialtask(char c);
run_result_t hw_serial_init(void *initial_state) {

  // init hardware, setup isr handler, masks
  SER_init(&serial_ipc);

  setPrinter(printchar_serialtask);

  // TODO: set interrupt handler (currently done at compile time)

  return PENDING;
}

ISR_void isr_on_hw_serial(void) { serial_isr_handler(); }

void printchar_serialtask(char c) {
  // push the char into serial buffer, interrupts and task polling should handle
  // the rest
  channel_send_uint16(&serial_ipc, c);
}
