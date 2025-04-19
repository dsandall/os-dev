#ifndef CHANNEL_ASYNC_H
#define CHANNEL_ASYNC_H

#include "async.h"
#include "channel.h"
#include <stdint.h>

typedef struct {
  ipc_channel_t *ch;
  uint8_t *out_byte;
  int *wake_flag;
} recv_state_t;

poll_result_t channel_recv_async(void *state);

#endif
