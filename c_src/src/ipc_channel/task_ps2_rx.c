#include "channel.h"
#include "freestanding.h"
#include "printlib.h"
#include "ps2_keyboard.h"
#include <stdint.h>

ipc_channel_t chan = {0};

uint8_t recv_buf = 0;
int wake_flags[MAX_TASKS] = {0};

run_result_t ps2_rx_task(void *s) {
  static recv_state_t recv_state = {
      .ch = &chan, .out_byte = &recv_buf, .wake_flag = &wake_flags[0]};

  if (channel_recv_interrupt_safe(recv_state.ch, recv_state.out_byte)) {
    // printk("channel recieved: %hx\n", recv_buf);
    isr_driven_keyboard(recv_buf);
    return PENDING;
  }
  return PENDING; // we want this to continue being called when it's turn on
                  // the scheduler arrives
}

// And the ISR

ISR_void isr_on_ps2_rx() {
  uint8_t byte = PS2_RX_wrap();
  channel_send(&chan, byte);
  wake_flags[0] = 1; // wake task 0
}
