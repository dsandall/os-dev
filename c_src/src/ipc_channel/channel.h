#ifndef CHANNEL_H
#define CHANNEL_H

#include "async.h"
#include "freestanding.h"
#include <stdint.h>

// Define the channel type
#define DEFINE_IPC_CHANNEL(name, type, size)                                   \
  typedef struct {                                                             \
    volatile type buffer[size];                                                \
    volatile uint16_t head;                                                    \
    volatile uint16_t tail;                                                    \
    const uint16_t capacity;                                                   \
  } name##_t

// Create a named instance
#define CREATE_IPC_CHANNEL(var_name, type, size)                               \
  type##_t var_name = {.head = 0, .tail = 0, .capacity = size}

// defining and allocating the ipc_channels
DEFINE_IPC_CHANNEL(ipc_channel_uint8, uint8_t, 16);
// WARN: sizes not really synced up
DEFINE_IPC_CHANNEL(ipc_channel_uint16, uint16_t, 32);
// WARN: sizes not really synced up

#define DEFINE_CHANNEL_FUNCS(type)                                             \
  static inline bool channel_send_##type(ipc_channel_##type##_t *ch,           \
                                         type##_t val) {                       \
    uint16_t next = (ch->head + 1) % ch->capacity;                             \
    if (next == ch->tail)                                                      \
      return false;                                                            \
    ch->buffer[ch->head] = val;                                                \
    ch->head = next;                                                           \
    return true;                                                               \
  }                                                                            \
                                                                               \
  static inline bool channel_recv_##type(ipc_channel_##type##_t *ch,           \
                                         type##_t *out) {                      \
    if (ch->tail == ch->head)                                                  \
      return false;                                                            \
    *out = ch->buffer[ch->tail];                                               \
    ch->tail = (ch->tail + 1) % ch->capacity;                                  \
    return true;                                                               \
  }

DEFINE_CHANNEL_FUNCS(uint8);
DEFINE_CHANNEL_FUNCS(uint16);

bool channel_recv_interrupt_safe(ipc_channel_uint8_t *ch, uint8_t *out_byte);
// bool channel_recv_interrupt_safe(ipc_channel_uint16_t *ch, uint16_t
// *out_byte);

#endif
