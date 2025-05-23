#include "async.h"
#include "freestanding.h"
#include "interrupts.h"
#include "kmalloc.h"
#include "multiboot.h"
#include "printer.h"
#include "rejigger_paging.h"
#include "vga_textbox.h"
#include "virtpage_alloc.h"
#include <stdint.h>

extern run_result_t ps2_rx_task(void *s);

extern run_result_t vga_task(void *initial_state);
extern run_result_t vga_task_init(void *initial_state);

extern run_result_t hw_int_task(void *initial_state);
extern run_result_t hw_int_task_init(void *initial_state);

extern run_result_t hw_serial_task(void *initial_state);
extern run_result_t hw_serial_init(void *initial_state);

extern void printchar_serialtask(char c);

void doubleprint(char c) {
  printchar_vgatask(c);
  printchar_serialtask(c);
}

void kernel_main() {

  recreate_gdt();

  // vga, so we can printf
  vga_task_init(NULL);
  tracek("printing some stuff on vga\n");

  // hw interrupts, so we can interact with I/O and handle exceptions
  hw_int_task_init(NULL);

  //// initialize hardware serial
  // hw_serial_init(NULL);
  // debugk("single print\n");

  // generate free memory list
  fiftytwo_card_pickup();
  testPageAllocator();
  regenerate_page_tables();
  breakpoint();
  testVirtPageAlloc();
  testKmalloc();

  // setup double printing
  setPrinter(doubleprint);
  tracek("doubleprint\n");

  // Prepare to enter the matrix (by that I mean the async polling system)
  spawn_task(vga_task, NULL, NULL);
  spawn_task(ps2_rx_task, NULL, NULL);
  spawn_task(hw_serial_task, NULL, NULL);

  RESUME(true);
  while (1) {
    run_tasks();
  }
}
