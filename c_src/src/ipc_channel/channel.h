#ifndef CHANNEL_H
#define CHANNEL_H

#include "__type_macros.h"
#include "async.h"
#include "freestanding.h"
#include "printlib.h"

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
//// Channel Configuration Goes Here
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////

#define PS2_CHANNEL_SIZE 16
#define VGA_CHANNEL_SIZE 32

// defining  the ipc_channels
DEFINE_IPC_CHANNEL_TYPE(ipc_channel_uint8, uint8_t, PS2_CHANNEL_SIZE);
DEFINE_IPC_CHANNEL_TYPE(ipc_channel_uint16, uint16_t, VGA_CHANNEL_SIZE);

// allocating the ipc_channels (done in each task)

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
///
/// - macro to define channel functions for various types
/// - we just assume this will be used with interrupts, better safe than sorry,
/// right?
/// - lots of unneccessary cli and sti calls, but its ok
///

#define DEFINE_CHANNEL_FUNCS(type)                                             \
  static inline bool channel_send_##type(ipc_channel_##type##_t *ch,           \
                                         type##_t val) {                       \
    ASM_CLI();                                                                 \
    uint16_t next = (ch->head + 1) % ch->capacity;                             \
    if (next == ch->tail) {                                                    \
      ASM_STI();                                                               \
      return false;                                                            \
    }                                                                          \
    ch->buffer[ch->head] = val;                                                \
    ch->head = next;                                                           \
    ASM_STI();                                                                 \
    return true;                                                               \
  }                                                                            \
                                                                               \
  static inline bool channel_recv_##type(ipc_channel_##type##_t *ch,           \
                                         type##_t *out) {                      \
    ASM_CLI();                                                                 \
    if (ch->tail == ch->head) {                                                \
      ASM_STI();                                                               \
      return false;                                                            \
    }                                                                          \
    *out = ch->buffer[ch->tail];                                               \
    ch->tail = (ch->tail + 1) % ch->capacity;                                  \
    ASM_STI();                                                                 \
    return true;                                                               \
  }

DEFINE_CHANNEL_FUNCS(uint8);
DEFINE_CHANNEL_FUNCS(uint16);
///
///
////////////////////////////////////////
////////////////////////////////////////
#endif
