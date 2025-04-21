#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include "channel.h"
#include "freestanding.h"
#include "ps2_8042.h"
#include <stdint.h>

// Basic Set 2 scancode to ASCII (partial for demonstration)
static const char scancode_set2_ascii[128] = {
    [0x1C] = 'A',  [0x32] = 'B',  [0x21] = 'C', [0x23] = 'D',  [0x24] = 'E',
    [0x2B] = 'F',  [0x34] = 'G',  [0x33] = 'H', [0x43] = 'I',  [0x3B] = 'J',
    [0x42] = 'K',  [0x4B] = 'L',  [0x3A] = 'M', [0x31] = 'N',  [0x44] = 'O',
    [0x4D] = 'P',  [0x15] = 'Q',  [0x2D] = 'R', [0x1B] = 'S',  [0x2C] = 'T',
    [0x3C] = 'U',  [0x2A] = 'V',  [0x1D] = 'W', [0x22] = 'X',  [0x35] = 'Y',
    [0x1A] = 'Z',

    [0x45] = '0',  [0x16] = '1',  [0x1E] = '2', [0x26] = '3',  [0x25] = '4',
    [0x2E] = '5',  [0x36] = '6',  [0x3D] = '7', [0x3E] = '8',  [0x46] = '9',

    [0x0E] = '`',  [0x4E] = '-',  [0x55] = '=', [0x5D] = '\\', [0x66] = '\b',
    [0x29] = ' ',  [0x0D] = '\t',
    [0x5A] = '\n', // Enter

    // Add more as needed
};

void init_PS2();
typedef uint16_t keyout_t;

#include "async.h"
typedef struct {
  run_result_t result;
  keyout_t keypress;
} keyout_result_t;

keyout_result_t ps2_state_machine_driver(uint8_t rx_byte);

#endif // PS2_KEYBOARD_H
