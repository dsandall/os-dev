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

// volatile int int_rx_flag;
// #d ef in e BLOCK_FOR_NEWSCAN() \
//  do { \
//    do { \
//      rx = PS2_RX(); \
//    } while (!int_rx_flag); \
//    /*check and clear flag*/ \
//    int_rx_flag = 0; \
//    oldrx = rx; \
//  } while (0)
//
// void bad_poll_for_keys() {
//   POLL_STATUS_WHILE((!stat.output_full));
//
//   uint8_t rx = PS2_RX();
//   uint8_t oldrx = ~rx;
//
//   while (1) {
//
//     BLOCK_FOR_NEWSCAN();
//
//     // rx in chunks of make,
//     // break make,
//     // or extended make
//     // or extended + break + make
//
//     if (rx == SC2_BREAK) {
//       // break make
//       BLOCK_FOR_NEWSCAN();
//       // printk("release %hx, %c\n", rx, scancode_ascii_map[rx]);
//     } else if (rx == SC2_EXTENDED) {
//       // ext (break?) make
//       BLOCK_FOR_NEWSCAN();
//
//       if (rx == SC2_BREAK) {
//         // ext break make
//         BLOCK_FOR_NEWSCAN();
//         // printk("release (ext) %hx\n", rx);
//       } else {
//         // ext make
//         // printk("press (ext) %hx\n", rx);
//         printk("%hx", rx);
//       }
//
//     } else {
//       // single make
//       // printk("press %hx, %c\n", rx, scancode_ascii_map[rx]);
//       printk("%c", scancode_ascii_map[rx]);
//     }
//
//     //__asm__("int $3"); // Breakpoint interrupt (crashes system)
//   }
// }

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

  print_statusreg();
};

// isr_rx:
//  if keeb: read ps2 port byte
//  set flag:

void isr_driven_keyboard() {
  uint8_t rx = PS2_RX();
  uint8_t oldrx = ~rx;

  // rx in chunks of make,
  // break make,
  // or extended make
  // or extended + break + make
  static enum { BLANK, BRK, EXT, EXTBRK } state;

  printk("thebuggg \n");
  switch (rx) {
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
    printk("state: %d\n", state);
    if (state == EXT) {
      printk("ext not handled\n");
      state = BLANK;
    } else if (state == BRK) {
      printk("releasing %c\n", scancode_ascii_map[rx]);
      state = BLANK;
    } else if (state == EXTBRK) {
      printk("extBRK not handled\n");
      state = BLANK;
    } else if (state == BLANK) {
      printk("blnk: %c\n", scancode_ascii_map[rx]);
      state = BLANK;
    }
    break;
  }
}
