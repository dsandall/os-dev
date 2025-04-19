#include "channel_async.h"

poll_result_t channel_recv_async(void *s) {
  recv_state_t *state = (recv_state_t *)s;

  if (*state->wake_flag || channel_recv(state->ch, state->out_byte)) {
    *state->wake_flag = 0;
    return READY;
  }

  return PENDING;
}
