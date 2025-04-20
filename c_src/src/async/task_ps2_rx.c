#include "async.h"
#include "channel.h"
#include "freestanding.h"
#include "printer.h"
#include "ps2_keyboard.h"
#include <stdint.h>

CREATE_IPC_CHANNEL_INSTANCE(ps2_ipc, ipc_channel_uint8, PS2_CHANNEL_SIZE);
extern ipc_channel_uint16_t vga_channel;

// Allocating for the task (should only be spawned once):
// - 1 channel (from IRQ PS2 -> this Task)
// - 1 byte rx buffer

run_result_t ps2_rx_task(void *initial_state) {

  uint8_t out_byte;
  if (channel_recv_uint8(&ps2_ipc, &out_byte)) {
    tracek("rx - %d\n", out_byte);
    // printk("channel recieved: %hx\n", recv_buf);
    ps2_state_machine_driver(out_byte, &vga_channel);
    return PENDING;
  }
  return PENDING; // we want this to continue being called when it's turn on
                  // the scheduler arrives
}

// And the ISR
ISR_void isr_on_ps2_rx() {
  uint8_t byte = PS2_RX();
  channel_send_uint8(&ps2_ipc, byte);
}
