#include "coop.h"
#include "doubly_linked.h"
#include "freestanding.h"
#include "printer.h"

extern ISR_void PROC_block_on_handler(SchedulerSlot **block_group,
                                      int enable_ints);
extern SchedulerSlot *keyboard_block_group;

ISR_void syscall_handler(uint64_t syscall_num) {
  if (syscall_num == SYSCALL_YIELD) {
    PROC_reschedule();
  } else if (syscall_num == SYSCALL_PROC_RUN) {
    PROC_run_handler();
  } else if (syscall_num == SYSCALL_KEXIT) {
    PROC_kexit_handler();
  } else if (syscall_num == SYSCALL_PROC_BLOCK_ON) {
    PROC_block_on_handler(&keyboard_block_group, 0);
  } else {
    ERR_LOOP();
  }
};
