#include "serial.h"
#include "channel.h"
#include "freestanding.h"
#include <stdint.h>

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

// pointer to IPC rx channel
static ipc_channel_uint16_t *serial_ipc_src;

/// Initializes COM1 and prepares the serial output driver.
/// Should be called once during OS startup.
void SER_init(ipc_channel_uint16_t *input_channel) {
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
  serial_ipc_src = input_channel;

  // Enable THRE and Line interrupts
  outb(COM1_BASE + REG_IER, 0x03);

  breakpoint();
  static char str[] = "Hello from serial!";
  for (int i = 0; i < (int)strlen(str); i++) {
    channel_send_uint16(input_channel, str[i]);
  }
};

void serial_try_send() {
  // peek at the channel
  uint16_t peek;
  if (channel_peek_uint16(serial_ipc_src, &peek)) {
    // try to send next byte
    if ((inb(COM1_BASE + REG_LSR) & REG_LSR_THRE)) {
      outb(COM1_BASE + REG_DATA, (uint8_t)peek);
    }
    // WARN: no way to know if theres a persistent error
  };
}

ISR_void serial_isr_handler() {
  uint8_t iir = inb(COM1_BASE + 2) & IIR_TYPE_MASK; // Get interrupt source

  if (iir == IIR_TX_EMPTY) {

    // recv the byte, send it nowhere
    uint16_t traaash;
    channel_recv_uint16(serial_ipc_src, &traaash);

    // then peek at the channel and try to send it again
    serial_try_send();

    return;
  }

  // Check if the interrupt was from a Line Status Error
  else if (iir == IIR_LINE_STAT) {
    // Read LSR to clear error conditions
    uint8_t lsr = inb(COM1_BASE + REG_LSR);
    // Handle as needed
    if (lsr & 0x02) {
      // Framing error or parity error
    }
    if (lsr & 0x01) {
      // Buffer overrun
    }
    ERR_LOOP();
  }

  else if (iir == 0x01) {
    // Spurious interrupt
    ERR_LOOP();
    tracek("spurious interruptus\n");
  }
}
