#include "channel.h"

bool channel_send(ipc_channel_t *ch, uint8_t byte) {
  uint16_t next_head = (ch->head + 1) % CHANNEL_CAPACITY;
  if (next_head == ch->tail) {
    return false; // buffer full
  }
  ch->buffer[ch->head] = byte;
  ch->head = next_head;
  return true;
}

bool channel_recv(ipc_channel_t *ch, uint8_t *out_byte) {
  if (ch->tail == ch->head) {
    return false; // buffer empty
  }
  *out_byte = ch->buffer[ch->tail];
  ch->tail = (ch->tail + 1) % CHANNEL_CAPACITY;
  return true;
}
bool channel_recv_interrupt_safe(ipc_channel_t *ch, uint8_t *out_byte) {
  __asm__("cli");
  if (ch->tail == ch->head) {
    __asm__("sti");
    return false; // buffer empty
  }
  *out_byte = ch->buffer[ch->tail];
  ch->tail = (ch->tail + 1) % CHANNEL_CAPACITY;
  __asm__("sti");
  return true;
}
