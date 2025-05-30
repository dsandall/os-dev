#include "channel.h"
#include "serial.h"

CREATE_IPC_CHANNEL_INSTANCE(
    serial_ipc, uint16,
    UINT16_CHANNEL_SIZE); // allocating the ipc_channels (done in each task)

void printchar_serialtask(char c);

void hw_serial_init() {
  SER_init(&serial_ipc);
  // setPrinter(printchar_serialtask);
};

void printchar_serialtask(char c) {
  // push the char into serial buffer, interrupts and task polling should handle
  // the rest
  channel_send_uint16(&serial_ipc, c);
}
