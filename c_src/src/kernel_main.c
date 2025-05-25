#include "coop.h"
#include "freestanding.h"
#include "interrupts.h"
#include "kmalloc.h"
#include "multiboot.h"
#include "printer.h"
#include "rejigger_paging.h"
#include "tasks.h"
#include "vga_textbox.h"

void doubleprint(char c) {
  printchar_vgatask(c);
  printchar_serialtask(c);
}

Textbox_t boxxy = {.x_corner = 58,
                   .y_corner = 2,
                   .width = 15,
                   .height = 20,
                   .cursor = (position_t){38, 2},
                   .fg = VGA_WHITE,
                   .bg = VGA_BLACK};

extern void run_snakes_wrapper(Textbox_t *boxxy);
void kernel_main() {

  recreate_gdt();

  // vga, so we can printf
  vga_task_init(NULL);
  tracek("printing some stuff on vga\n");

  // hw interrupts, so we can interact  I/O and handle exceptions
  hw_int_task_init(NULL);

  // generate free memory list
  fiftytwo_card_pickup();
  regenerate_page_tables();

  run_snakes_wrapper(&boxxy);
  PROC_run();
  tracek("continueinggieanstirdean\n");

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
