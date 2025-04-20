#include "channel.h"
#include "freestanding.h"
#include "printlib.h"
#include "ps2_keyboard.h"
#include <stdint.h>

// Allocating for the task (should only be spawned once):
// - 1 channel (from IRQ PS2 -> this Task)
// - 1 byte rx buffer

typedef ipc_channel_uint8_t ps2_chan_t;
typedef struct {
  ps2_chan_t *ch;
} ps2_taskstate_t;

CREATE_IPC_CHANNEL(ps2_ipc, ipc_channel_uint8, 16);

ps2_taskstate_t ps2_ts = {.ch = &ps2_ipc};

run_result_t ps2_rx_task(void *initial_state) {

  uint8_t out_byte;
  if (channel_recv_interrupt_safe(ps2_ts.ch, &out_byte)) {
    // printk("channel recieved: %hx\n", recv_buf);
    isr_driven_keyboard(out_byte);
    return PENDING;
  }
  return PENDING; // we want this to continue being called when it's turn on
                  // the scheduler arrives
}

// And the ISR
ISR_void isr_on_ps2_rx() {
  uint8_t byte = PS2_RX_wrap();
  channel_send_uint8(ps2_ts.ch, byte);
}
