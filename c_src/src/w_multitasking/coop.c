#include "coop.h"
#include "doubly_linked.h"
#include "freestanding.h"
#include "kmalloc.h"
#include "paging.h"
#include "printer.h"

bool need_init = true;

Process boot_thread = {.context = {0}, 0, false};
SchedulerSlot boot_slot = {
    .proc = &boot_thread, .next = &boot_slot, .prev = &boot_slot};

SchedulerSlot *scheduler_current = &boot_slot;
SchedulerSlot *scheduler_on_deck = &boot_slot;

void PROC_add_to_scheduler(Process *t) {
  SchedulerSlot *new_slot = (SchedulerSlot *)kmalloc(sizeof(SchedulerSlot));
  *new_slot = (SchedulerSlot){t, NULL, NULL};
  insert(scheduler_current, new_slot);
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
  t->tid = current_pid;
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
  if (scheduler_current->proc->dead) {
    SchedulerSlot *next;
    if ((next = separate_from(scheduler_current)) != NULL) {
      // dead and has a diff proc (remove and reschedule)
      tracek("KEXIT - current is dead, removing and rescheduling\n");
      scheduler_on_deck = next;
    } else {
      // dead and has only itself (remove and goto boot_thread)
      tracek("YIELD - all threads dead, returning to boot_thread\n");
      scheduler_on_deck = &boot_slot;
    }
  } else {
    if (scheduler_current->next == scheduler_current) {
      tracek("YIELD - returning to yielder\n");
    } else {
      // tracek("YIELD - passing control\n");
    }
    scheduler_on_deck = scheduler_current->next;
  }
  return;
};

ISR_void PROC_run_handler(void) {
  tracek("PROC_RUN - removed boot thread from scheduler\n");
  scheduler_on_deck = separate_from(&boot_slot);
  ASSERT(scheduler_on_deck); // you probably should have other threads when
                             // calling this
  return;
};

/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
// PROC_create_kthread(some_thing, (void *)5);
// PROC_create_kthread(some_thing, (void *)3);
// PROC_create_kthread(some_thing, (void *)2);
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
