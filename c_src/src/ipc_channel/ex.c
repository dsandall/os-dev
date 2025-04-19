#include "channel_async.h"
#include "printlib.h"
#include <stdint.h>
extern void isr_driven_keyboard(uint8_t);

ipc_channel_t chan = {0};

uint8_t recv_buf = 0;
int wake_flags[MAX_TASKS] = {0};

poll_result_t my_task(void *s) {
  static recv_state_t recv_state = {
      .ch = &chan, .out_byte = &recv_buf, .wake_flag = &wake_flags[0]};

  if (channel_recv_async(&recv_state) == READY) {
    // Do something with recv_buf
    printk("skibidi: %hx\n", recv_buf);
    isr_driven_keyboard(recv_buf);
    // return READY;
    return PENDING; // we want this to continue being called when it's turn on
                    // the scheduler arrives
  }

  return PENDING;
}

int async_main(void) {
  spawn_task(my_task, NULL);

  __asm__("sti"); // enable interrupts

  while (1) {
    run_tasks();

    // Simulate ISR firing
    // isr_on_data_rx(0x41); // 'A'
  }
}
