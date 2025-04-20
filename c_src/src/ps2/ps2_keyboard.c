#include "ps2_keyboard.h"
#include "channel.h"
#include "freestanding.h"
#include "keyboard_scancodes.h"
#include "printlib.h"
#include "ps2_8042.h"
#include <stdint.h>

extern void init_PS2_8042(void);
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

  //__asm__("int $3");

  if (lazytx(0xFF) != 0xFA) {
    asm("hlt");
  } else if (PS2_RX() != 0xAA) {
    asm("hlt");
  }

  //  // test echo
  //  if (lazytx(0xEE) != 0xEE) {
  //    asm("hlt");
  //  }

  //__asm__("int $3");

  POLL_STATUS_WHILE(stat.input_full);
  PS2_TX(0xF0);
  POLL_STATUS_WHILE(stat.input_full);
  PS2_TX(0x02);
  POLL_STATUS_WHILE(!stat.output_full);
  if (PS2_RX() != 0xFA) {
    asm("hlt");
  }

  //__asm__("int $3");

  // enable it
  if (lazytx(0xF4) != 0xFA) {
    asm("hlt");
  }

  print_statusreg();
};

// isr_rx:
//  if keeb: read ps2 port byte
//  set flag:

uint8_t PS2_RX_wrap() {
  uint8_t rx = PS2_RX();
  return rx;
}

void isr_driven_keyboard(uint8_t rx_byte,
                         ipc_channel_uint16_t *text_out_channel) {

  // rx in chunks of make,
  // break make,
  // or extended make
  // or extended + break + make
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
    printk("ext not handled\n");
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
      printk("extBRK not handled\n");
      state = BLANK;
    } else if (state == BLANK) {

      uint16_t key_tx = scancode_ascii_map[rx_byte];
      bool ret = channel_send_uint16(text_out_channel, key_tx);
      if (ret == false) {
        ERR_LOOP();
      }
      // printk("%c", scancode_ascii_map[rx_byte]);
      state = BLANK;
    }
    break;
  }
}
