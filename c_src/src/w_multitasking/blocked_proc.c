#include "doubly_linked.h"

// keyboard driver -
// use queue to pass data from PS2 IRQ to user function that reads keyboard
//
// note that we are now tracking processes that are scheduled, and processes
// that are not scheduled
//
// a single process may be in multiple or no queues, or the scheduler pq
//
// "blocking" a process moves it from the scheduler to the proc queue
//
//
// Contexts and Functions:
// Blocking and NonBlocking Funcs:
//    Nonblocking: normal funcs
//    Blocking: funcs that could end up waiting for some external event
//    (possibly forever!)
// Blocking and Nonblocking Contexts:
//    Blocking contexts: normal funcs
//    Non Blocking contexts: sections of code that should probably never be
//    blocked! (ISRs, printf, etc. stuff that should always happen smoothly, and
//    give back control)

// one queue for each block event (serial response, ps2 response, disk response)

// move proc to pq from scheduler
// (beware race condition, blocking a p while it recieves an int unblock)
void PROC_block_on(SchedulerSlot *pq, int enable_ints) {};

// remove 1 or more from pq
void PROC_unblock_all(SchedulerSlot *pq) {};
void PROC_unblock_head(SchedulerSlot *pq) {};

// init pq
void PROC_init_queue(SchedulerSlot *pq) {};
