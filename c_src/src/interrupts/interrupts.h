#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define FAULT_GENERAL_PROTECTION 0x0D
#define FAULT_DOUBLE 0x08
#define FAULT_PAGE 0x0E

#define STACK_SIZE (16384) // 4 KiB stack size

void recreate_gdt();

#endif
