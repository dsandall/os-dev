#include "channel.h"
#include <stdint.h>

#define UINT8_CHANNEL_SIZE 700
#define UINT16_CHANNEL_SIZE 700

// defining  the ipc_channels
// DEFINE_IPC_CHANNEL_TYPE(ipc_channel_uint8, uint8_t, UINT8_CHANNEL_SIZE);
// DEFINE_IPC_CHANNEL_TYPE(ipc_channel_uint16, uint16_t, UINT16_CHANNEL_SIZE);

// Define the channel type
// #define DEFINE_IPC_CHANNEL_TYPE(name, type, size) \
//  typedef struct { \
//    volatile type buffer[size]; \
//    volatile uint16_t head; \
//    volatile uint16_t tail; \
//    const uint16_t capacity; \
//  } name##_t

typedef struct {
  volatile uint8_t buffer[UINT8_CHANNEL_SIZE];
  volatile uint32_t head;
  volatile uint32_t tail;
  const uint32_t capacity;
} ipc_channel_uint8_t;

typedef struct {
  volatile uint16_t buffer[UINT16_CHANNEL_SIZE];
  volatile uint32_t head;
  volatile uint32_t tail;
  const uint32_t capacity;
} ipc_channel_uint16_t;

// Create a named instance (must be expanded AFTER channel functions are
// defined)
#define CREATE_IPC_CHANNEL_INSTANCE(var_name, type, size)                      \
  typedef type##_t var_name##_data_t;                                          \
  ipc_channel_##type##_t var_name = {.head = 0, .tail = 0, .capacity = size}
