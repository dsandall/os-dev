#include <stdint.h>

// Define the channel type
#define DEFINE_IPC_CHANNEL_TYPE(name, type, size)                              \
  typedef struct {                                                             \
    volatile type buffer[size];                                                \
    volatile uint16_t head;                                                    \
    volatile uint16_t tail;                                                    \
    const uint16_t capacity;                                                   \
  } name##_t

// Create a named instance (must be expanded AFTER channel functions are
// defined)
#define CREATE_IPC_CHANNEL_INSTANCE(var_name, type, size)                      \
  typedef type##_t var_name##_data_t;                                          \
  ipc_channel_##type##_t var_name = {.head = 0, .tail = 0, .capacity = size}
