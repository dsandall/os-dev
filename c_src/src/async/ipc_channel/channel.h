#ifndef CHANNEL_H
#define CHANNEL_H

#include "__type_macros.h"
#include "freestanding.h"
#include "printer.h"

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
    bool state = PAUSE_INT();                                                  \
    uint16_t next = (ch->head + 1) % ch->capacity;                             \
    if (next == ch->tail) {                                                    \
      RESUME(state);                                                           \
      return false;                                                            \
    }                                                                          \
    ch->buffer[ch->head] = val;                                                \
    ch->head = next;                                                           \
    RESUME(state);                                                             \
    return true;                                                               \
  }                                                                            \
                                                                               \
  static inline bool channel_recv_##type(ipc_channel_##type##_t *ch,           \
                                         type##_t *out) {                      \
    bool state = PAUSE_INT();                                                  \
    if (ch->tail == ch->head) {                                                \
      RESUME(state);                                                           \
      return false;                                                            \
    }                                                                          \
    *out = ch->buffer[ch->tail];                                               \
    ch->tail = (ch->tail + 1) % ch->capacity;                                  \
    RESUME(state);                                                             \
    return true;                                                               \
  }                                                                            \
  static inline bool channel_peek_##type(ipc_channel_##type##_t *ch,           \
                                         type##_t *out) {                      \
    bool state = PAUSE_INT();                                                  \
    if (ch->tail == ch->head) {                                                \
      RESUME(state);                                                           \
      return false;                                                            \
    }                                                                          \
    *out = ch->buffer[ch->tail];                                               \
    RESUME(state);                                                             \
    return true;                                                               \
  }

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
//// Channel Configuration Goes Here
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////

/// the funcs
DEFINE_CHANNEL_FUNCS(uint8);
DEFINE_CHANNEL_FUNCS(uint16);

// the instances
#endif
