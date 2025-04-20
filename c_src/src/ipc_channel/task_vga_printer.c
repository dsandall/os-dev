#include "channel.h"
#include "freestanding.h"
#include "printlib.h"
#include "ps2_keyboard.h"
#include "vgalib.h"
#include <stdint.h>

typedef ipc_channel_uint8_t vga_chan_t;
typedef struct {
  vga_chan_t *ch;
} vga_taskstate_t;

// Allocating for the task (should only be spawned once):
// - 1 channel (from PS2_rx -> this task, the vga buffer)
vga_taskstate_t vga_ts;

run_result_t vga_task(void *initial_state) {

  uint8_t send_to_vga;
  if (channel_recv_uint8(vga_ts.ch, &send_to_vga)) {
    // printk("channel recieved: %hx\n", recv_buf);
    // //TODO: vga printing here
    return PENDING;
  }

  return PENDING; // we want this to continue being called when it's turn on
                  // the scheduler arrives
}

//// Calling Func example:
// ISR_void isr_on_ps2_rx() {
//   uint8_t byte = PS2_RX_wrap();
//   channel_send(vga_ts.ch, byte);
// }
