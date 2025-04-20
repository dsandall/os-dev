#include "channel.h"
#include "freestanding.h"
#include "printlib.h"
#include "ps2_keyboard.h"
#include "vgalib.h"
#include <stdint.h>

CREATE_IPC_CHANNEL_INSTANCE(vga_channel, ipc_channel_uint16, VGA_CHANNEL_SIZE);

// Allocating for the task (should only be spawned once):
// - 1 channel (from PS2_rx -> this task, the vga buffer)

run_result_t vga_task(void *initial_state) {

  uint16_t send_to_vga;
  if (channel_recv_uint16(&vga_channel, &send_to_vga)) {
    tracek("send - %d\n", send_to_vga);
    // printk("channel recieved: %hx\n", recv_buf);
    // //TODO: vga printing here

    tracek("printing:\n");
    printk("%c", send_to_vga);
    tracek("end printing:\n");
    return PENDING;
  }

  return PENDING; // we want this to continue being called when it's turn on
                  // the scheduler arrives
}
