#include "freestanding.h"

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

#define PS2_DATA_IN_OUT 0x60
#define PS2_RX() ((uint8_t)inb(PS2_DATA_IN_OUT))
#define PS2_TX(x)                                                              \
  do {                                                                         \
    outb(PS2_DATA_IN_OUT, x);                                                  \
  } while (0)

#define PS2_STATUS_COMMAND 0x64
#define PS2_STATUS() ((status_register_t)inb(PS2_STATUS_COMMAND))
#define PS2_COMMAND(x)                                                         \
  do {                                                                         \
    outb(PS2_STATUS_COMMAND, x);                                               \
  } while (0)

#define POLL_STATUS_WHILE(x)                                                   \
  do {                                                                         \
    status_register_t stat;                                                    \
    do {                                                                       \
      stat = PS2_STATUS();                                                     \
    } while (x);                                                               \
  } while (0)

void init_PS2();
void init_PS2_8042(void);
void get_statusreg();
