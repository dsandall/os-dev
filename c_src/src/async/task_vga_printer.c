#include "async/async.h"
#include "channel.h"
#include "vga_textbox.h"
#include <stdint.h>

CREATE_IPC_CHANNEL_INSTANCE(vga_channel, ipc_channel_uint16, VGA_CHANNEL_SIZE);

// Allocating for the task (should only be spawned once):
// - 1 channel (from PS2_rx -> this task, the vga buffer)

run_result_t vga_task(void *initial_state) {

  uint16_t send_to_vga;
  if (channel_recv_uint16(&vga_channel, &send_to_vga)) {
    printk("%c", send_to_vga);
  }

  return PENDING;
}

run_result_t vga_task_init(void *initial_state) {
  static Textbox_t main_box = {.x_corner = 8,
                               .y_corner = 2,
                               .width = 60,
                               .height = 20,
                               .cursor = (position_t){8, 2}};
  VGA_textbox_init(&main_box);
  return PENDING;
}
