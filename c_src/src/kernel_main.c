#include "coop.h"
#include "doubly_linked.h"
#include "freestanding.h"
#include "interrupts.h"
#include "kmalloc.h"
#include "multiboot.h"
#include "printer.h"
#include "rejigger_paging.h"
#include "tasks.h"
#include "vga_textbox.h"
#include "vgalib.h"

extern void some_thing();
extern void hw_int_init();
extern void run_snakes_wrapper();
extern void ps2_rx_task(Textbox_t *);
extern void vga_init();
extern void printchar_defaultvga(char);

SchedulerSlot *keyboard_block_group;

void doubleprint(char c) {
  printchar_defaultvga(c);
  printchar_serialtask(c);
}

extern void serial_try_send();
void proc_dummy() {
  while (1) {
    serial_try_send(); // WARN : should only be needed if it gets lost...
    yield();
    // breakpoint();
  }
}

static Textbox_t *default_box;
static Textbox_t *blue_bar;
extern void print_char_tobox_immediate(char c, Textbox_t *box);

void printchar_defaultvga(char c) {
  print_char_tobox_immediate(c, default_box);
}

void vga_init() {

  // how to set up windows and textboxes
  static Textbox_t bar = {.x_corner = 0,
                          .y_corner = VGA_HEIGHT - 3,
                          .width = VGA_WIDTH,
                          .height = 2,
                          .cursor = (position_t){0, VGA_HEIGHT - 3},
                          .fg = VGA_WHITE,
                          .bg = VGA_BLUE};

  static Textbox_t main_box = {.x_corner = 2,
                               .y_corner = 2,
                               .width = 56,
                               .height = 20,
                               .cursor = (position_t){2, 2},
                               .fg = VGA_WHITE,
                               .bg = VGA_DARK_GREY};

  setPrinter((printfunction)&printchar_defaultvga);

  default_box = &bar;
  blue_bar = &bar;
  clear_Textbox(&bar);

  default_box = &main_box;
  clear_Textbox(&main_box);

  VGA_printTest();
}

void kernel_main() {

  recreate_gdt();

  // vga, so we can printf
  vga_init(NULL);
  tracek("printing some stuff on vga\n");

  // hw interrupts, so we can interact  I/O and handle exceptions
  hw_int_init();

  // generate free memory list
  fiftytwo_card_pickup();
  regenerate_page_tables();
  // run_snakes_wrapper();
  // PROC_run();

  //// initialize hardware serial
  hw_serial_init(NULL);
  // debugk("single print\n");

  // setup double printing
  // setPrinter(doubleprint);
  // debugk("doubleprint\n");

  // Prepare to enter the matrix (by that I mean the async polling system)
  // spawn_task(vga_task, NULL, NULL);
  // spawn_task(ps2_rx_task, NULL, NULL);
  // spawn_task(hw_serial_task, NULL, NULL);

  // PROC_create_kthread(proc_dummy, NULL);
  RESUME(true);

  PROC_create_kthread((kproc_t)ps2_rx_task, blue_bar);
  PROC_create_kthread(some_thing, (void *)1);
  PROC_run();

  while (1) {
    // run_tasks();
  }
}
