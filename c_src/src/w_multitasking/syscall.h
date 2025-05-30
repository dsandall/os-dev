#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "freestanding.h"

#define SYSCALL_PROC_BLOCK_ON 50

#define SYSCALL_PROC_RUN 78
#define SYSCALL_YIELD 77
#define SYSCALL_KEXIT 76

static void syscall(uint64_t syscall_number) {
  __asm__ volatile("mov %[input], %%rax"
                   :
                   : [input] "r"(syscall_number)
                   : "rax");
  __asm__("int $0x80");
}

#endif
