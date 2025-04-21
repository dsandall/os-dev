#include "async.h"
#include "channel.h"
#include "freestanding.h"
#include "printer.h"
#include "ps2_keyboard.h"
#include <stdint.h>

CREATE_IPC_CHANNEL_INSTANCE(ps2_ipc, uint8, UINT8_CHANNEL_SIZE);
extern ipc_channel_uint16_t serial_ipc;
extern ipc_channel_uint16_t vga_ipc;

// Allocating for the task (should only be spawned once):
// - 1 channel (from IRQ PS2 -> this Task)
// - 1 byte rx buffer

run_result_t ps2_rx_task(void *initial_state) {

  uint8_t out_byte;
  if (channel_recv_uint8(&ps2_ipc, &out_byte)) {
    tracek("rx - %d\n", out_byte);
    // printk("channel recieved: %hx\n", recv_buf);

    // potentially rx from state machine

    keyout_result_t nextchar = ps2_state_machine_driver(out_byte);

    if (nextchar.result == DATA) {
      // send on vga and serial channels
      channel_send_uint16(&serial_ipc, nextchar.keypress);
      // channel_send_uint16(&vga_ipc, nextchar.keypress);// not needed, print
      // handler writes directly
    }
  }

  return PENDING; // we want this to continue being called when it's turn on
                  // the scheduler arrives
}

// And the ISR
ISR_void isr_on_ps2_rx() {
  uint8_t byte = PS2_RX();
  channel_send_uint8(&ps2_ipc, byte);
}
