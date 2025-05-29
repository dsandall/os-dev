#include "async/async.h"
#include "channel.h"
#include "printer.h"
#include "vga_textbox.h"

CREATE_IPC_CHANNEL_INSTANCE(vga_ipc, uint16, UINT16_CHANNEL_SIZE);

// VGA has one channel, and manages data into the VGA buffer
//
// It also supports a print handler

async_run_result_t vga_task(void *initial_state) {

  uint16_t send_to_vga;
  if (channel_recv_uint16(&vga_ipc, &send_to_vga)) {
    printchar_vgatask(send_to_vga);
  }
  return PENDING;
}

// pointer to a box and cursor struct
static Textbox_t *default_box;

// print handler, no channel from printer - writes directly to the vga textbox
extern void print_char_tobox_immediate(char c, Textbox_t *box);
void printchar_vgatask(char c) { print_char_tobox_immediate(c, default_box); }

async_run_result_t vga_task_init(void *initial_state) {

  // how to set up windows and textboxes

  static Textbox_t bar = {.x_corner = 0,
                          .y_corner = VGA_HEIGHT - 3,
                          .width = VGA_WIDTH,
                          .height = 2,
                          .cursor = (position_t){0, VGA_HEIGHT - 3},
                          .fg = VGA_BLUE,
                          .bg = VGA_BLUE};

  static Textbox_t main_box = {.x_corner = 8,
                               .y_corner = 2,
                               .width = 30,
                               .height = 20,
                               .cursor = (position_t){8, 2},
                               .fg = VGA_WHITE,
                               .bg = VGA_DARK_GREY};

  setPrinter((printfunction)&printchar_vgatask);

  default_box = &bar;
  clear_Textbox(&bar);

  default_box = &main_box;
  clear_Textbox(&main_box);

  VGA_printTest();

  return PENDING;
}
