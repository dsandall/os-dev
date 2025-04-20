#ifndef CHANNEL_H
#define CHANNEL_H

#include "async.h"
#include "freestanding.h"
#include <stdint.h>

#define CHANNEL_CAPACITY 16

typedef struct {
  volatile uint8_t buffer[CHANNEL_CAPACITY];
  volatile uint16_t head; // write index
  volatile uint16_t tail; // read index
} ipc_channel_t;

bool channel_recv(ipc_channel_t *ch, uint8_t *out_byte);
bool channel_send(ipc_channel_t *ch, uint8_t byte);
bool channel_recv_interrupt_safe(ipc_channel_t *ch, uint8_t *out_byte);

/////////////////
/////////////////

typedef struct {
  ipc_channel_t *ch;
  uint8_t *out_byte;
  int *wake_flag;
} recv_state_t;

#endif
