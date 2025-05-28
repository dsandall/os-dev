#include "coop.h"
#include "freestanding.h"
#include "kmalloc.h"
#include "paging.h"
#include "printer.h"
#include <errno.h>
#include <stdint.h>

bool need_init = true;
Process boot_thread = {.context = {0}, 0, &boot_thread};
Process *glbl_thread_current = &boot_thread;
Process *glbl_thread_on_deck = &boot_thread;

void PROC_add_to_scheduler(Process *t) {
  // insert in list
  Process *list = glbl_thread_current->next;

  if (list == NULL) {
    tracek("LIST is NULL!\n");
    list = glbl_thread_current;
  }

  glbl_thread_current->next = t;
  ASSERT(t);

  t->next = list;
  ASSERT(t->next);
};

Process *PROC_create_kthread(kproc_t entry_point, void *arg) {
  // create new thread

  // TODO: allocate somewhere other than the heap region
  Process *t = (Process *)kmalloc(sizeof(Process)).point;

  // allocate a new stack (kernel stack)
  void *stack_top = (void *)(kmalloc(PAGE_SIZE * 10).point);
  stack_top += PAGE_SIZE * 10; // WARN: suboptimal

  // init thread context
  //    entry_point func should be called when this thread is scheduled

  static uint64_t current_pid = 1;
  t->pid = current_pid++;
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
    if (glbl_thread_current->next == NULL) {
      ERR_LOOP();
    } else if (glbl_thread_current->next == glbl_thread_current) {
      // no other threads are available
      if (glbl_thread_current->dead) {
        tracek("YIELD - all threads dead, returning to boot_thread\n");
        ERR_LOOP();
      } else {
        tracek("YIELD - returning to yielder\n");
      }
    } else {
      // other threads available
      if (((Process *)glbl_thread_current->next)->dead) {
        // prune threads
        tracek("YIELD - next is dead, removing and rescheduling\n");
        glbl_thread_current->next =
            ((Process *)glbl_thread_current->next)->next;
        goto try;
      } else {
        tracek("YIELD - selecting next\n");
        glbl_thread_on_deck = glbl_thread_current->next;
      }
    }
};

ISR_void syscall_handler(uint64_t syscall_num) {
  if (syscall_num == SYSCALL_YIELD) {
    PROC_reschedule();
  } else if (syscall_num == SYSCALL_KEXIT) {
    tracek("exiting\n");
    glbl_thread_current->dead = true;
    // TODO: free stuff
    PROC_reschedule();
  } else {
    ERR_LOOP();
  }
};

void yield(void) { syscall(SYSCALL_YIELD); };

void kexit(void) { syscall(SYSCALL_KEXIT); };

void inner(uint64_t arg) {
  uint64_t counter = arg;
  while (1) {
    if (counter) {
      tracek("inner_hello (%lx/%lx)\n", counter--, arg);
      yield();
    } else {
      tracek("goodbye! (%lx/%lx)\n", counter, arg);
      kexit();
    }
  }
}

void some_thing(void *arg) {
  tracek("celebrate (%lx)\n", (uint64_t)arg);
  // yield();
  inner((uint64_t)arg);
};

void PROC_run(void) {

  boot_thread.next = &boot_thread;

  PROC_create_kthread(some_thing, (void *)5);
  PROC_create_kthread(some_thing, (void *)2);

  while (1) {
    tracek("og says bye\n");
    kexit();
  }
};
