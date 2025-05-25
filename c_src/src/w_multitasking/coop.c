#include "coop.h"
#include "paging.h"
#include <stdint.h>
typedef struct {
  phys_addr cr3;
  struct {
    // (deepest in stack/chronologically pushed first/highest address)
    // automatic stack stored
    uint64_t ss;     // stack segment
    uint64_t rsp;    // stack pointer
    uint16_t rflags; // reg flags (overflow, etc) (popped by iretq)
    uint16_t cs;     // code segment (popped  by iretq)
    uint64_t rip;    // instruction pointer (popped  by iretq)
    // regular registers
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t ds; // mostly regular
    uint64_t es; // mostly regular
    uint64_t fs;
    uint64_t gs;
    // (top of stack/chronologically pushed last/lowest address)
  };
} context_t;

typedef struct {
  uint64_t tid;
  context_t context;
} thread_t;

thread_t *glbl_thread_current;
thread_t *glbl_thread_next;

// Called in a loop at the end of kmain. This drives the entire
// multi-tasking system. The next thread gets selected and run. Threads can
// yield, exit, etc. If no thread is able to run then PROC_run() returns.
void PROC_run(void) {

};

// Adds a new thread to the multi-tasking system. This requires allocating a new
// stack in the virtual address space and initializing the thread's context such
// that the entry_point function gets executed the next time this thread is
// scheduled. This function does not actually schedule the thread.
void PROC_create_kthread(kproc_t entry_point, void *arg) {
  // create new thread

  // allocate a new stack (kernel stack)

  // init thread context
  //    entry_point func should be called when this thread is scheduled

};

// Selects the next thread to run. It must select the "thread" that called
// PROC_run if not other threads are available to run. This function does not
// actually perform a context switch.
void PROC_reschedule(void) {
  // scheduler (round robin)

  // defaults to the original kernel thread (the one that called proc run)

  // does not actually switch
};

// Invokes the scheduler and passes control to the next eligible thread. It is
// possible to return to the thread that called yield if no other eligible
// threads exist. To make context switch implementation more consistent, and
// your life slightly easier, I suggest making yield() a trap that calls the
// actual yield implementation. This way the stack frame is always the same and
// you can reuse the assembly interrupt handler you have already written. An
// example implementation of yield which just triggers trap number 123 is:
//
// static inline void yield(void) { asm volatile("INT $123"); }
void yield(void) {
  // passes control to next thread (called by any (kernel?) thread)
  // can return to itself
  // should be a trap instruction that calls the actual implementation
  //    this is to have a consistent yield stack (tss mentioned)
};

// Exits and destroys all the state of the thread that calls kexit. Needs to run
// the scheduler to pick another process. I also suggest you use a trap-based
// implementation AND the IST mechanism so that the trap handler runs on a
// different stack. Running on a different stack makes it possible to free the
// thread's stack without pulling the rug out from under yourself.
void kexit(void) {
  // destroys the calling thread

  // ends by calling scheduler

  // should also be trap based
  // (IST - interrupt stack table)
  // has own stack, so you don't destroy yourself while still existing

};
