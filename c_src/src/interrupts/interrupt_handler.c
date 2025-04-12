#include "printlib.h"

// this is the driver for the 8259 PIC HW Interrupt Controller
// https://wiki.osdev.org/8259_PIC#Programming_with_the_8259_PIC

void interrupt_handler(void) { printk("interrupting cow goes moooo\n"); }
