#include "async.h"
#include "freestanding.h"
#include "printer.h"

extern run_result_t ps2_rx_task(void *s);

extern run_result_t vga_task(void *initial_state);
extern run_result_t vga_task_init(void *initial_state);

extern run_result_t hw_int_task(void *initial_state);
extern run_result_t hw_int_task_init(void *initial_state);

void kernel_main() {

  // vga, so we can printf
  spawn_task(vga_task, NULL, vga_task_init);

  // hw interrupts, so we can interact with I/O and handle exceptions
  spawn_task(NULL, NULL, hw_int_task_init);

  // now ps2 is set up and you should be rxing keeb interrupts
  spawn_task(ps2_rx_task, NULL, NULL);

  // Prepare to enter the matrix (by that I mean the async polling system)
  while (1) {
    run_tasks();
  }
}
