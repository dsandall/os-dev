#ifndef SERIAL_H
#define SERIAL_H

#include "channel.h"
#include "freestanding.h"

#define COM1_BASE 0x3F8

#define REG_DATA 0        // Transmit/Receive
#define REG_IER 1         // Interrupt Enable Register
#define REG_IIR 2         // Interrupt Identification Register (read only)
#define REG_LSR 5         // Line Status Register
#define REG_LSR_THRE 0x20 // Transmitter Holding Register Empty
#define REG_LSR_ERR 0x1E  // Bits 1-4 indicate errors

#define IIR_TYPE_MASK 0x0F
#define IIR_TX_EMPTY 0x02
#define IIR_LINE_STAT 0x06

void SER_init(ipc_channel_uint16_t *input_channel);

/// Queues data to be written to the serial port (non-blocking).
void SER_printc(char c);

void serial_try_send();

// manages transactions with serial hardware, called by interrupt or polled
void serial_isr_handler(void);
#endif
