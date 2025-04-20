#include "channel.h"

/////////////////////////////////////////////////////////////////////////
// Cooperative multitasking:
// just send or recv

// bool channel_send(ipc_channel_t *ch, uint8_t byte) {
//   uint16_t next_head = (ch->head + 1) % ch->capacity;
//   if (next_head == ch->tail) {
//     return false; // buffer full
//   }
//   ch->buffer[ch->head] = byte;
//   ch->head = next_head;
//   return true;
// }
//
// bool channel_recv(ipc_channel_t *ch, uint8_t *out_byte) {
//   if (ch->tail == ch->head) {
//     return false; // buffer empty
//   }
//   *out_byte = ch->buffer[ch->tail];
//   ch->tail = (ch->tail + 1) % ch->capacity;
//   return true;
// }

/////////////////////////////////////////////////////////////////////////
// Recieving from an uncooperative source:
// Call recieve, but block interrupts while you access the shared memory
//
// bool channel_recv_interrupt_safe(ipc_channel_ *ch, uint8_t *out_byte) {
//  __asm__("cli");
//  bool ret = channel_recv_uint8(ch, out_byte);
//  __asm__("sti");
//  return ret;
//}
