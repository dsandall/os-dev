#include "ps2_keyboard.h"
#include "keyboard_scancodes.h"
#include "ps2_8042.h"
#include <stdint.h>

extern void init_PS2_8042(void);
extern void printk(const char *fmt, ...);
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
};

uint8_t lazytx(uint8_t tx) {
  POLL_STATUS_WHILE(stat.input_full);
  PS2_TX(tx);
  POLL_STATUS_WHILE(!stat.output_full);
  return PS2_RX();
}

#define BLOCK_FOR_NEWSCAN()                                                    \
  do {                                                                         \
    do {                                                                       \
      rx = PS2_RX();                                                           \
    } while (rx == oldrx);                                                     \
    oldrx = rx;                                                                \
  } while (0)

void bad_poll_for_keys() {
  POLL_STATUS_WHILE((!stat.output_full));

  uint8_t rx = PS2_RX();
  uint8_t oldrx = ~rx;

  while (1) {

    BLOCK_FOR_NEWSCAN();

    // rx in chunks of make,
    // break make,
    // or extended make
    // or extended + break + make

    if (rx == SC2_BREAK) {
      // break make
      BLOCK_FOR_NEWSCAN();
      // printk("release %hx, %c\n", rx, scancode_ascii_map[rx]);
    } else if (rx == SC2_EXTENDED) {
      // ext (break?) make
      BLOCK_FOR_NEWSCAN();

      if (rx == SC2_BREAK) {
        // ext break make
        BLOCK_FOR_NEWSCAN();
        // printk("release (ext) %hx\n", rx);
      } else {
        // ext make
        // printk("press (ext) %hx\n", rx);
        printk("%hx", rx);
      }

    } else {
      // single make
      // printk("press %hx, %c\n", rx, scancode_ascii_map[rx]);
      printk("%c", scancode_ascii_map[rx]);
    }

    //__asm__("int $3"); // Breakpoint interrupt (crashes system)
  }
}

void init_PS2_keyboard() {
  if (lazytx(0xFF) != 0xFA) {
    asm("hlt");
  } else if (PS2_RX() != 0xAA) {
    asm("hlt");
  }

  //  // test echo
  //  if (lazytx(0xEE) != 0xEE) {
  //    asm("hlt");
  //  }

  POLL_STATUS_WHILE(stat.input_full);
  PS2_TX(0xF0);
  POLL_STATUS_WHILE(stat.input_full);
  PS2_TX(0x02);
  POLL_STATUS_WHILE(!stat.output_full);
  if (PS2_RX() != 0xFA) {
    asm("hlt");
  }

  // enable it
  if (lazytx(0xF4) != 0xFA) {
    asm("hlt");
  }

  while (1) {
    print_statusreg();
    bad_poll_for_keys();
  }

  return;
};
