#include "coop.h"
#include "freestanding.h"
#include "interrupts.h"
#include "kmalloc.h"
#include "multiboot.h"
#include "printer.h"
#include "rejigger_paging.h"
#include "tasks.h"
#include "vga_textbox.h"

extern void hw_int_init();
extern void run_snakes_wrapper();

void doubleprint(char c) {
  printchar_vgatask(c);
  printchar_serialtask(c);
}

void kernel_main() {

  recreate_gdt();

  // vga, so we can printf
  vga_task_init(NULL);
  tracek("printing some stuff on vga\n");

  // hw interrupts, so we can interact  I/O and handle exceptions
  hw_int_init();

  // generate free memory list
  fiftytwo_card_pickup();
  regenerate_page_tables();

  run_snakes_wrapper();
  PROC_run();

  //// initialize hardware serial
  hw_serial_init(NULL);
  debugk("single print\n");

  // setup double printing
  setPrinter(doubleprint);
  debugk("doubleprint\n");

  // Prepare to enter the matrix (by that I mean the async polling system)
  spawn_task(vga_task, NULL, NULL);
  spawn_task(ps2_rx_task, NULL, NULL);
  spawn_task(hw_serial_task, NULL, NULL);

  RESUME(true);

  while (1) {
    run_tasks();
  }
}
