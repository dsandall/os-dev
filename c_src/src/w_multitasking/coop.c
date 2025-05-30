#include "coop.h"
#include "freestanding.h"
#include "kmalloc.h"
#include "paging.h"
#include "printer.h"
#include <errno.h>
#include <iso646.h>
#include <stdint.h>

bool need_init = true;

Process boot_thread = {.context = {0}, 0, false};
SchedulerSlot boot_slot = {&boot_thread, &boot_slot};

SchedulerSlot *scheduler_current = &boot_slot;
SchedulerSlot *scheduler_on_deck = &boot_slot;

void PROC_add_to_scheduler(Process *t) {
  // insert in list

  // if (glbl_thread_current == &boot_thread) {
  //   tracek(
  //       "initializing scheduler with first process, removing
  //       boot_thread!\n");
  //   ASSERT(glbl_thread_on_deck == &boot_thread);
  //   glbl_thread_current = t;
  //   glbl_thread_on_deck = t;
  //   t->next = t;
  //   return;
  // }

  SchedulerSlot *list = scheduler_current->next;

  if (list == NULL) {
    ERR_LOOP();
    tracek("WARNING: LIST is NULL!\n");
  }

  SchedulerSlot *new_slot = (SchedulerSlot *)kmalloc(sizeof(SchedulerSlot));
  *new_slot = (SchedulerSlot){.proc = t, .next = list};
  scheduler_current->next = new_slot;

  ASSERT(new_slot->next);
};

Process *PROC_create_kthread(kproc_t entry_point, void *arg) {
  // create new thread

  // TODO: allocate somewhere other than the heap region
  Process *t = (Process *)kmalloc(sizeof(Process));

  // allocate a new stack (kernel stack)
  void *stack_base = (void *)(kmalloc(PAGE_SIZE * 10));
  void *stack_top = stack_base + PAGE_SIZE * 10; // stack grows down

  // Reserve space for fake return address
  stack_top -= sizeof(void *);
  *(uint64_t *)stack_top = (uint64_t)kexit;

  // init thread context
  //    entry_point func should be called when this thread is scheduled

  static int current_pid = 1;
  t->pid = current_pid;
  t->mypid = current_pid++;
  t->context.rip = entry_point;
  t->context.rsp = stack_top;
  t->context.cs = 8;
  t->context.rflags = 0x202;
  t->context.rdi = (uint64_t)arg; // TODO: support better args
  t->dead = false;

  PROC_add_to_scheduler(t);

  return t;
};

void PROC_reschedule(void) {
  try:
    asm("nop");

  ASSERT(scheduler_current->next != NULL);

  if (scheduler_current->next == &boot_slot) {
    tracek("first time returning to boot thread, remove from list\n");
    scheduler_current->next = scheduler_current->next->next;

    // could handle this differently, but is a weird edge case
    ASSERT(scheduler_current->next != &boot_slot);
    goto try;

  } else if (scheduler_current->next == scheduler_current) {
    // no other threads are available
    if (scheduler_current->proc->dead) {
      tracek("YIELD - all threads dead, returning to boot_thread\n");
      scheduler_on_deck = &boot_slot;
      return;
    } else {
      tracek("YIELD - returning to yielder\n");
      return;
    }
  } else {
    // other threads available
    if (scheduler_current->next->proc->dead) {
      // prune threads
      tracek("YIELD - next is dead, removing and rescheduling\n");
      scheduler_current->next = scheduler_current->next->next;
      goto try;
    } else {
      tracek("YIELD - selecting next\n");
      scheduler_on_deck = scheduler_current->next;
    }
  }
};

ISR_void syscall_handler(uint64_t syscall_num) {
  if (syscall_num == SYSCALL_YIELD) {
    PROC_reschedule();
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

void yield(void) { syscall(SYSCALL_YIELD); };

__attribute__((noreturn)) void kexit(void) {
  tracek("setting up exit for pid %d, slot %p\n",
         scheduler_current->proc->mypid, scheduler_current);
  syscall(SYSCALL_KEXIT);
  ERR_LOOP();
};

void inner(uint64_t arg) {
  uint64_t counter = arg;
  while (counter) {
    tracek("inner_hello (%lu/%lu)\n", counter--, arg);
    yield();
  }
  tracek("goodbye! (%lu/%lu)\n", counter, arg);
  // kexit(); // implicit
}

void some_thing(void *arg) {
  tracek("celebrate (%lu)\n", (uint64_t)arg);
  // yield();
  inner((uint64_t)arg);
};

void PROC_run(void) {

  // PROC_create_kthread(some_thing, (void *)5);
  // PROC_create_kthread(some_thing, (void *)3);
  // PROC_create_kthread(some_thing, (void *)2);

  tracek("og says bye\n");

  yield();

  tracek(
      "all threads successfully exited, and we have returned to boot thread\n");
};
