#ifndef SERIAL_H
#define SERIAL_H

#include "channel.h"

void SER_init(ipc_channel_uint16_t *input_channel);

/// Queues data to be written to the serial port (non-blocking).
void SER_printc(char c);

void serial_try_send();

// manages transactions with serial hardware, called by interrupt or polled
void serial_isr_handler(void);
#endif
