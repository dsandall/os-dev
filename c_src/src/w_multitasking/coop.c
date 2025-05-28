#include "coop.h"
#include "freestanding.h"
#include "kmalloc.h"
#include "paging.h"
#include "printer.h"
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
  void *stack_top =
      (void *)(kmalloc(PAGE_SIZE * 4).raw + PAGE_SIZE * 3); // WARN: suboptimal

  // init thread context
  //    entry_point func should be called when this thread is scheduled

  static uint64_t current_pid = 1;
  *t = (Process){0};          // WARN:debug
  *(uint64_t *)stack_top = 0; // WARN:debug
  t->pid = current_pid++;
  t->context.rip = entry_point;
  t->context.rsp = stack_top;
  t->context.cs = 8;
  t->context.rflags = 0x202;
  t->context.rdi = (uint64_t)arg; // TODO: support better args

  PROC_add_to_scheduler(t);

  return t;
};

void PROC_reschedule(void) {

  if (glbl_thread_current->next == NULL) {
    ERR_LOOP();
  } else if (glbl_thread_current->next == glbl_thread_current) {
    tracek("returning to yielded thread\n");
    // glbl_thread_on_deck->context.cs = 8; // WARN: debug
  } else {
    tracek("selecting next thread\n");
    glbl_thread_on_deck = glbl_thread_current->next;
    // glbl_thread_on_deck->context.cs = 8; // WARN: debug
  }
};

ISR_void syscall_handler(uint64_t syscall_num) {
  if (syscall_num == SYSCALL_YIELD) {
    tracek("yielding\n");
    PROC_reschedule();
  } else if (syscall_num == SYSCALL_KEXIT) {
    tracek("exiting\n");
    ERR_LOOP();

    // Process **pp = &glbl_thread_current;

    // ASSERT(*pp != NULL);

    // while (*pp) {
    //   if (*pp == glbl_thread_current) {
    //     *pp = glbl_thread_current->next;
    //     break;
    //   }
    //   pp = &(*pp)->next;
    // }

  } else {
    ERR_LOOP();
  }
};

void yield(void) { syscall(SYSCALL_YIELD); };

void kexit(void) { syscall(SYSCALL_KEXIT); };

void some_thing(void *arg) {
  while (1) {
    tracek("celebrate (%lx)\n", (uint64_t)arg);
    yield();
  }
};

void PROC_run(void) {

  boot_thread.next = &boot_thread;

  // tracek("og says hi\n");
  // yield();
  // tracek("og says hi\n");
  // yield();
  // tracek("og says hi\n");
  // yield();

  PROC_create_kthread(some_thing, (void *)0x11);
  PROC_create_kthread(some_thing, (void *)0x22);

  while (1) {
    tracek("og says hi\n");

    yield();
  }
};
