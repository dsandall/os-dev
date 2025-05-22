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
  spawn_task(vga_task, NULL, vga_task_init);
  tracek("printing some stuff on vga\n");

  // hw interrupts, so we can interact with I/O and handle exceptions
  spawn_task(NULL, NULL, hw_int_task_init);

  __asm__("int $0x20");

  // generate free memory list
  fiftytwo_card_pickup();
  regenerate_page_tables();

  // test demand paging and vpage allocator
  uint64_t *somedata = (uint64_t *)MMU_alloc_page().raw;
  *somedata = 0xBEEF;
  if (*somedata != (uint64_t)0xBEEF) {
    debugk("allocated memory not working\n");
  }

  // now ps2 is set up and you should be rxing keeb interrupts
  spawn_task(ps2_rx_task, NULL, NULL);
  tracek("hardware ps2 enabled\n");

  // initialize hardware serial
  spawn_task(hw_serial_task, NULL, hw_serial_init);
  tracek("single print\n");

  // setup double printing
  setPrinter(doubleprint);
  tracek("doubleprint meeee\n");

  // Prepare to enter the matrix (by that I mean the async polling system)
  RESUME(true);

  // test kmalloc
  virt_addr_t allocated_pointer = kmalloc(69);
  uint64_t *someotherdata = (uint64_t *)(allocated_pointer.raw);
  *someotherdata = 0xBEEF;
  if (*someotherdata != (uint64_t)0xBEEF) {
    debugk("kmalloc not working\n");
  }

  kfree(allocated_pointer);

  while (1) {
    run_tasks();
  }
}
