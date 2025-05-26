#include "coop.h"
#include "freestanding.h"
#include "kmalloc.h"
#include "paging.h"
#include "printer.h"
#include "tester.h"
#include <stdint.h>

bool need_init = true;
Process boot_thread;
Process *glbl_thread_current = &boot_thread;
Process *glbl_thread_next = &boot_thread;

uint64_t current_pid = 1;

extern void print_current_thread() {
  context_t g = glbl_thread_current->context;
  tracek("rip:%p, rflags:%lx, cs:%lx,\n", g.rip, g.rflags, g.cs);
};

void PROC_add_to_scheduler(Process *t) {
  glbl_thread_next = t; // TODO:
};

// Adds a new thread to the multi-tasking system. This requires allocating a new
// stack in the virtual address space and initializing the thread's context such
// that the entry_point function gets executed the next time this thread is
// scheduled. This function does not actually schedule the thread.

__attribute__((aligned(4096))) uint8_t stack_test[16384];

void PROC_create_kthread(kproc_t entry_point, void *arg) {
  // create new thread

  // TODO: allocate somewhere other than the heap region
  Process *t = (Process *)kmalloc(sizeof(Process)).point;

  // allocate a new stack (kernel stack)
  void *stack_top =
      (void *)(kmalloc(PAGE_SIZE * 4).raw + PAGE_SIZE * 3); // WARN: suboptimal

  // init thread context
  //    entry_point func should be called when this thread is scheduled
  *t = (Process){0};
  t->pid = current_pid++;
  t->context.rip = entry_point; // TODO: add args
  t->context.rsp = stack_top;
  t->context.cs = 8;
  t->context.rflags = 0x202;

  // immediately add to scheduler
  PROC_add_to_scheduler(t);
};

void PROC_reschedule(void) {
  // TODO:
  // scheduler (round robin)

  // defaults to the original kernel thread (the one that called proc run)

  // does not actually switch
};

void yield(void) {
  // passes control to next thread (called by any (kernel?) thread)
  // can return to itself
  // should be a trap instruction that calls the actual implementation
  //    this is to have a consistent yield stack (tss mentioned)
  __asm__("int $0x80");
};

ISR_void yield_actual(void) { PROC_reschedule(); }

void kexit(void) {
  // destroys the calling thread

  // ends by calling scheduler

  // should also be trap based
  // (IST - interrupt stack table)
  // has own stack, so you don't destroy yourself while still existing

};

void some_thing() {
  breakpoint();

  while (1) {
    tracek("celebrate\n");
  }

  kexit();
};

void PROC_run(void) {
  // glbl_thread_current = (thread_t *)kmalloc(sizeof(thread_t)).point;
  //*(uint64_t *)glbl_thread_current = 0;

  PROC_create_kthread(some_thing, NULL);

  while (1) {
    yield();
  }
};
