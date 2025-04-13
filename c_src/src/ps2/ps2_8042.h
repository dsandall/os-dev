#include "freestanding.h"

#define PS2_DATA_IN_OUT 0x60
#define PS2_RX() ((uint8_t)inb(PS2_DATA_IN_OUT))
#define PS2_TX(x)                                                              \
  do {                                                                         \
    outb(x, PS2_DATA_IN_OUT);                                                  \
  } while (0)

#define PS2_STATUS_COMMAND 0x64
#define PS2_STATUS() ((status_register_t)inb(PS2_STATUS_COMMAND))
#define PS2_COMMAND(x)                                                         \
  do {                                                                         \
    outb(x, PS2_STATUS_COMMAND);                                               \
  } while (0)

#define POLL_STATUS_WHILE(x)                                                   \
  do {                                                                         \
    uint16_t timer = 10000; /*block for a little while*/                       \
    status_register_t stat;                                                    \
    do {                                                                       \
      stat = PS2_STATUS();                                                     \
      timer--;                                                                 \
      if (!timer) {                                                            \
        printk("sadge\n");                                                     \
      }                                                                        \
    } while (x);                                                               \
  } while (0)

typedef union {
  uint8_t raw;
  struct {
    unsigned output_full : 1;
    unsigned input_full : 1;
    unsigned sys_flag : 1;
    unsigned to_controller : 1;
    unsigned : 2;
    unsigned err_timeout : 1;
    unsigned err_parity : 1;
  } __attribute__((packed));
} status_register_t;

typedef union {
  uint8_t raw;
  struct {
    unsigned en_int_p1 : 1;
    unsigned en_int_p2 : 1;
    unsigned sys_flag : 1;
    unsigned : 1;
    unsigned disable_clk_p1 : 1;
    unsigned disable_clk_p2 : 1;
    unsigned en_p1_translation : 1;
    unsigned : 1;
  } __attribute__((packed));
} controller_configuration_byte_t;

/*
typedef union {
  uint8_t raw;
  struct {
    unsigned : 4;
    unsigned output_full_p1 : 1;
    unsigned output_full_p2 : 1;
    unsigned : 2;
  } __attribute__((packed));
} controller_output_port_t;
*/

void init_PS2();
status_register_t print_statusreg();
