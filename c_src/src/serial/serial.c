#include "serial.h"
#include "channel.h"
#include "freestanding.h"
#include "printer.h"
#include <stdint.h>

// pointer to IPC rx channel
static ipc_channel_uint8_t *serial_ipc_recv;

// Print func wrapper for the printer library
/// Queues data to be written to the serial port (non-blocking).
void SER_printc(char c) { channel_send_uint8(serial_ipc_recv, (uint8_t)c); };

/// Initializes COM1 and prepares the serial output driver.
/// Should be called once during OS startup.
void SER_init(ipc_channel_uint8_t *input_channel) {
  outb(COM1_BASE + REG_IER, 0x00); // Disable all interrupts
  outb(COM1_BASE + 3, 0x80);       // Enable DLAB
  outb(COM1_BASE + 0, 0x03);       // Baud rate divisor low byte (38400)
  outb(COM1_BASE + 1, 0x00);       // Baud rate divisor high byte
  outb(COM1_BASE + 3, 0x03);       // 8 bits, no parity, one stop bit
  outb(COM1_BASE + 2,
       0xC7); // FIFO control: enable FIFO, clear, 14-byte threshold
  outb(COM1_BASE + 4, 0x0B); // Modem control: IRQs enabled, RTS/DSR set

  // Clear line status
  (void)inb(COM1_BASE + REG_LSR);

  // set the channel to read from
  serial_ipc_recv = input_channel;

  // Enable THRE and Line interrupts
  outb(COM1_BASE + REG_IER, 0x03);
};

// helper funcs
static inline bool serial_can_send(void) {
  return (inb(COM1_BASE + REG_LSR) & REG_LSR_THRE) != 0;
}
// static inline void serial_hw_send(uint8_t byte) {
//   outb(COM1_BASE + REG_DATA, byte);
// }

static bool acked_flag = false; // WARN: static state only in isr
void serial_try_send() {
  // peek at the channel
  uint8_t peek;
  if (channel_peek_uint8(serial_ipc_recv, &peek)) {
    // try to send next byte
    if (!serial_can_send()) {
      ERR_LOOP();
    }
    outb(COM1_BASE + REG_DATA, peek);
    acked_flag = false;
  };
}

ISR_void serial_isr_handler() {
  uint8_t iir = inb(COM1_BASE + 2) & IIR_TYPE_MASK; // Get interrupt source

  if (iir == IIR_TX_EMPTY) {
    acked_flag = true;

    if (!serial_can_send()) {
      ERR_LOOP();
    }

    // recv the byte, send it nowhere
    uint8_t traaash;
    channel_recv_uint8(serial_ipc_recv, &traaash);

    // then peek at the channel and try to send it again
    serial_try_send();

    return;
  }

  // Check if the interrupt was from a Line Status Error
  else if (iir == IIR_LINE_STAT) {
    // Read LSR to clear error conditions (e.g., overrun, framing error)
    uint8_t lsr = inb(COM1_BASE + REG_LSR);
    // Handle specific error conditions as needed
    if (lsr & 0x02) {
      // Framing error or parity error: Handle accordingly
    }
    if (lsr & 0x01) {
      // Buffer overrun: Handle accordingly
    }
    ERR_LOOP();
  }

  else if (iir == 0x01) {
    // Spurious interrupt
    // Optionally log or ignore
    tracek("spurious interruptus\n");
  }
}

//// Serial Logic
//// called by isr, or by polling
// void serial_tx_handler(void) {
//   static size_t pos = 0;
//
//   // If no message in progress, try to get one
//   if (!current_msg) {
//     static char temp_buf[128]; // or whatever max size you expect
//     size_t i = 0;
//     uint8_t byte;
//
//     while (i < sizeof(temp_buf) &&) {
//       temp_buf[i++] = byte;
//     }
//
//     if (i > 0) {
//       temp_buf[i] = '\0'; // optional, if using C-string ops
//       current_msg = temp_buf;
//       pos = 0;
//     }
//   }
//
//   // Now send next byte
//   if (current_msg && current_msg[pos]) {
//     if (serial_can_send()) { // Check LSR THRE
//       serial_hw_send(current_msg[pos++]);
//     }
//   }
//
//   // Done with message?
//   if (current_msg && current_msg[pos] == '\0') {
//     current_msg = NULL;
//   }
// }
