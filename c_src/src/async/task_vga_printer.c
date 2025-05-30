#include "async/async.h"
#include "channel.h"
#include "printer.h"
#include "vga_textbox.h"

CREATE_IPC_CHANNEL_INSTANCE(vga_ipc, uint16, UINT16_CHANNEL_SIZE);

// VGA has one channel, and manages data into the VGA buffer
//
// It also supports a print handler

// pointer to a box and cursor struct

extern void printchar_defaultvga(char c);
static async_run_result_t vga_task(void *initial_state) {
  uint16_t send_to_vga;

  if (channel_recv_uint16(&vga_ipc, &send_to_vga)) {
    printchar_defaultvga(send_to_vga);
  }
  return PENDING;
}
