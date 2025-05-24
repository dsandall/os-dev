#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include "async.h"
#include "channel.h"
#include "freestanding.h"

void init_PS2();

#include "async.h"
typedef struct {
  async_run_result_t result;
  uint16_t keypress;
} keyout_result_t;

keyout_result_t ps2_state_machine_driver(uint8_t rx_byte);

#endif // PS2_KEYBOARD_H
