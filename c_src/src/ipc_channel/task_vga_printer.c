#include "channel.h"
#include "freestanding.h"
#include "printlib.h"
#include "ps2_keyboard.h"
#include "vgalib.h"
#include <stdint.h>

typedef ipc_channel_uint8_t vga_chan_t;

CREATE_IPC_CHANNEL(vga_channel, ipc_channel_uint8, 16);

// Allocating for the task (should only be spawned once):
// - 1 channel (from PS2_rx -> this task, the vga buffer)

run_result_t vga_task(void *initial_state) {

  uint8_t send_to_vga;
  if (channel_recv_uint8(&vga_channel, &send_to_vga)) {
    // printk("channel recieved: %hx\n", recv_buf);
    // //TODO: vga printing here

    printk("%c\n", send_to_vga);

    return PENDING;
  }

  return PENDING; // we want this to continue being called when it's turn on
                  // the scheduler arrives
}
