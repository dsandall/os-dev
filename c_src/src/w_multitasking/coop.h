#ifndef COOP_H
#define COOP_H

#include "freestanding.h"

typedef void (*kproc_t)(void *);

// Called in a loop at the end of kmain. This drives the entire
// multi-tasking system. The next thread gets selected and run. Threads can
// yield, exit, etc. If no thread is able to run then PROC_run() returns.
void PROC_run(void);

// Adds a new thread to the multi-tasking system. This requires allocating a new
// stack in the virtual address space and initializing the thread's context such
// that the entry_point function gets executed the next time this thread is
// scheduled. This function does not actually schedule the thread.
void PROC_create_kthread(kproc_t entry_point, void *arg);

// Selects the next thread to run. It must select the "thread" that called
// PROC_run if not other threads are available to run. This function does not
// actually perform a context switch.
void PROC_reschedule(void);

// Invokes the scheduler and passes control to the next eligible thread. It is
// possible to return to the thread that called yield if no other eligible
// threads exist. To make context switch implementation more consistent, and
// your life slightly easier, I suggest making yield() a trap that calls the
// actual yield implementation. This way the stack frame is always the same and
// you can reuse the assembly interrupt handler you have already written. An
// example implementation of yield which just triggers trap number 123 is:
//
// static inline void yield(void) { asm volatile("INT $123"); }
void yield(void);

// Exits and destroys all the state of the thread that calls kexit. Needs to run
// the scheduler to pick another process. I also suggest you use a trap-based
// implementation AND the IST mechanism so that the trap handler runs on a
// different stack. Running on a different stack makes it possible to free the
// thread's stack without pulling the rug out from under yourself.
void kexit(void);

#endif
