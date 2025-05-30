#include "ps2_keyboard.h"
#include "async.h"
#include "freestanding.h"
#include "keyboard_scancodes.h"
#include "printer.h"
#include "ps2_8042.h"
#include <stdint.h>

void init_PS2_keyboard(void);

typedef union {
  uint8_t raw;
  struct {
    unsigned rate : 5;
    unsigned delay : 2;
    unsigned : 1;
  } __attribute__((packed));
} typematic_t;

void init_PS2() {
  init_PS2_8042();
  init_PS2_keyboard();
  tracek("keyboard initialization complete\n");
};

uint8_t lazytx(uint8_t tx) {
  POLL_STATUS_WHILE(stat.input_full);
  PS2_TX(tx);
  POLL_STATUS_WHILE(!stat.output_full);
  return PS2_RX();
}

void init_PS2_keyboard() {

  if (lazytx(0xFF) != 0xFA) {
    ERR_LOOP();
  } else if (PS2_RX() != 0xAA) {
    ERR_LOOP();
  }

  // test echo
  if (lazytx(0xEE) != 0xEE)
    ERR_LOOP();

  POLL_STATUS_WHILE(stat.input_full);
  PS2_TX(0xF0);
  POLL_STATUS_WHILE(stat.input_full);
  PS2_TX(0x02);
  POLL_STATUS_WHILE(!stat.output_full);

  if (PS2_RX() != 0xFA)
    ERR_LOOP();

  // enable it
  if (lazytx(0xF4) != 0xFA)
    ERR_LOOP();

  get_statusreg();
};

keyout_result_t ps2_state_machine_driver(uint8_t rx_byte) {

  // rx in chunks of make,
  // break make,
  // or extended make
  // or extended + break + make
  keyout_result_t result = {PENDING, 0};

  static enum { BLANK, BRK, EXT, EXTBRK } state;

  switch (rx_byte) {
  case SC2_BREAK:
    if (state == EXT) {
      state = EXTBRK;
    } else {
      state = BRK;
    }
    break;
  case SC2_EXTENDED:
    tracek("ext not handled\n");
    state = EXT;
    break;
  default:
    // case "make":
    // printk("state: %d\n", state);
    if (state == EXT) {
      // printk("ext not handled\n");
      state = BLANK;
    } else if (state == BRK) {
      // printk("releasing %c\n", scancode_ascii_map[rx_byte]);
      state = BLANK;
    } else if (state == EXTBRK) {
      tracek("extBRK not handled\n");
      state = BLANK;
    } else if (state == BLANK) {

      result.keypress = scancode_ascii_map[rx_byte];
      result.result = DATA;
      state = BLANK;
    }
    break;
  }

  return result;
}
