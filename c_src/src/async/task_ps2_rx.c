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

// TODO: just something silly to do when you get a keypress
void ps2_onkeypressevent(keyout_t k) {
  // send on vga and serial channels
  static bool send_both = false;
  if (k == ' ') {
    send_both = !send_both;
  }

  channel_send_uint16(&serial_ipc, k);
  if (send_both) {
    channel_send_uint16(&vga_ipc, k);
  }
};

run_result_t ps2_rx_task(void *initial_state) {

  uint8_t out_byte;
  if (channel_recv_uint8(&ps2_ipc, &out_byte)) {
    tracek("rx - %d\n", out_byte);
    // potentially rx from state machine
    keyout_result_t nextchar = ps2_state_machine_driver(out_byte);

    switch (nextchar.result) {
    case PENDING:
    case DEAD:
      break;
    case DATA:
      ps2_onkeypressevent(nextchar.keypress);
      break;
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
