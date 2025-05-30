#include "doubly_linked.h"
#include "freestanding.h"
#include "syscall.h"
#include "vga_textbox.h"

// keyboard driver -
// use queue to pass data from PS2 IRQ to user function that reads keyboard
//
// note that we are now tracking processes that are scheduled, and processes
// that are not scheduled
//
// a single process may be in multiple or no queues, or the scheduler pq
//
// "blocking" a process moves it from the scheduler to the proc queue
//
//
// Contexts and Functions:
// Blocking and NonBlocking Funcs:
//    Nonblocking: normal funcs
//    Blocking: funcs that could end up waiting for some external event
//    (possibly forever!)
// Blocking and Nonblocking Contexts:
//    Blocking contexts: normal funcs
//    Non Blocking contexts: sections of code that should probably never be
//    blocked! (ISRs, printf, etc. stuff that should always happen smoothly, and
//    give back control)

// one queue for each block event (serial response, ps2 response, disk response)

// move proc to pq from scheduler
// (beware race condition, blocking a p while it recieves an int unblock)

// NOTE: this could also be a macro, where the caller passes in a boolean
// condition to be checked
//
extern SchedulerSlot *keyboard_block_group;

ISR_void PROC_block_on_handler(SchedulerSlot **block_group, int enable_ints) {
  SchedulerSlot *rest = separate_from(scheduler_current);
  insert_end(block_group, scheduler_current);
  setOnDeck(rest);
};

// remove 1 or more from pq
ISR_void PROC_unblock_head(SchedulerSlot **block_group) {
  if (block_group[0]) {
    SchedulerSlot *rest =
        separate_from(block_group[0]); // extract first from block group
    insert_end(&scheduler_current,
               block_group[0]); // WARN: not sure about this one
    *block_group = rest;
    // no context swap yet, let scheduler handle it
  } else {
    tracek("nothing in block group yet\n");
  }
};

//////
// PS2
//////

#include "channel.h"
#include "freestanding.h"
#include "printer.h"
#include "ps2_8042.h"
#include "ps2_keyboard.h"
#include <stdint.h>

CREATE_IPC_CHANNEL_INSTANCE(ps2_ipc, uint8, UINT8_CHANNEL_SIZE);
extern ipc_channel_uint16_t serial_ipc;
// extern ipc_channel_uint16_t vga_ipc;

// Allocating for the task (should only be spawned once):
// - 1 channel (from IRQ PS2 -> this Task)
// - 1 byte rx buffer
//
// pointer to a box and cursor struct

extern void print_char_tobox_immediate(char c, Textbox_t *box);
void ps2_rx_task(Textbox_t *blue_bar) {

  uint8_t out_byte;
  bool int_state;
  while (1) {

    // WARN: race conditions
    tracek("ps2 blocking\n");
    breakpoint();
    syscall(SYSCALL_PROC_BLOCK_ON);
    tracek("ps2 unblocked\n");

    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);
    print_char_tobox_immediate('k', blue_bar);

    int_state = PAUSE_INT();

    if (channel_recv_uint8(&ps2_ipc, &out_byte)) {
      // tracek("rx - %d\n", out_byte);
      // potentially rx from state machine
      keyout_result_t nextchar = ps2_state_machine_driver(out_byte);
      char k = nextchar.keypress;

      switch (nextchar.result) {
      case DATA:
        print_char_tobox_immediate(k, blue_bar);
        channel_send_uint16(&serial_ipc, k);
        break;
      case PENDING:
      case DEAD:
        break;
      }
    } else {
      tracek("PRINTER CALLED WITH NONE\n");
    }

    RESUME(int_state);
  }
}

ISR_void isr_on_ps2_rx() {
  // And the ISR
  // tracek("PS2 ISR rx\n");
  channel_send_uint8(&ps2_ipc, PS2_RX());
  PROC_unblock_head(&keyboard_block_group);
}
