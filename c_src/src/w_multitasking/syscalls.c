#include "coop.h"
#include "doubly_linked.h"
#include "freestanding.h"
#include "printer.h"

ISR_void syscall_handler(uint64_t syscall_num) {
  if (syscall_num == SYSCALL_YIELD) {
    PROC_reschedule();
  } else if (syscall_num == SYSCALL_PROC_RUN) {
    PROC_run_handler();
  } else if (syscall_num == SYSCALL_KEXIT) {
    tracek("exiting\n");
    breakpoint();
    scheduler_current->proc->dead = true;
    // TODO: free stuff
    PROC_reschedule();
  } else {
    ERR_LOOP();
  }
};
