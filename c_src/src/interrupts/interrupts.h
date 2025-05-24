#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define FAULT_GENERAL_PROTECTION 0x0D
#define FAULT_DOUBLE 0x08
#define FAULT_PAGE 0x0E

void recreate_gdt();
void init_IDT();

#endif
