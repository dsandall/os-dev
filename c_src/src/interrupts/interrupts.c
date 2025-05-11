#include "interrupts.h"
#include "freestanding.h"
#include "pic_8259.h"
#include "printer.h"

// https://wiki.osdev.org/8259_PIC#Programming_with_the_8259_PIC
// void interrupt_handler(void) { printk("interrupting cow goes moooo\n"); }

///////////////////////////////////////
/// The actual handler
///////////////////////////////////////

ISR_void exception_handler(uint32_t vector);

ISR_void asm_int_handler(uint16_t *ptr) {
  uint16_t vector = *ptr;
  if (vector < 0x20) {
    // exceptions
    exception_handler(vector);
    return;
  } else if (vector <= 0x2F) {
    // Hardware IRQs (0x20 - 0x2F)
    PIC_common_handler(vector);
    return;
  } else {
    printk("Unknown CPU exception: 0x%x\n", vector);
    ERR_LOOP();
  }
}

///////////
// Interrupt Handling
///////////

ISR_void isr_on_ps2_rx();
void PIC_sendEOI(uint8_t irq);
extern void isr_on_hw_serial();

ISR_void PIC_common_handler(uint32_t vector) {
  switch (vector) {
  case 0x20:
    // printk("Timer interrupt (IRQ0)\n");
    break;
  case 0x21:
    // printk("Keyboard interrupt (IRQ1)\n");
    isr_on_ps2_rx();
    break;
  case 0x22:
    printk("Cascade (IRQ2, usually unused)\n");
    ERR_LOOP();
  case 0x23:
    printk("COM2 (IRQ3)\n");
    ERR_LOOP();
  case 0x24:
    // printk("COM1 (IRQ4)\n");
    isr_on_hw_serial();
    break;
  case 0x25:
    printk("LPT2 / floppy (IRQ5)\n");
    ERR_LOOP();
  case 0x26:
    printk("Floppy / sound card (IRQ6)\n");
    ERR_LOOP();
  case 0x27:
    printk("LPT1 / spurious (IRQ7)\n");
    ERR_LOOP();
  case 0x28:
    printk("RTC / CMOS (IRQ8)\n");
    ERR_LOOP();
  case 0x29:
    printk("ACPI / legacy SCSI (IRQ9)\n");
    ERR_LOOP();
  case 0x2A:
    printk("Unused / network / USB (IRQ10)\n");
    ERR_LOOP();
  case 0x2B:
    printk("Unused / SCSI (IRQ11)\n");
    ERR_LOOP();
  case 0x2C:
    printk("PS/2 mouse (IRQ12)\n");
    ERR_LOOP();
  case 0x2D:
    printk("FPU / coprocessor / IRQ13\n");
    ERR_LOOP();
  case 0x2E:
    printk("Primary ATA (IRQ14)\n");
    ERR_LOOP();
  case 0x2F:
    printk("Secondary ATA (IRQ15)\n");
    ERR_LOOP();
  }
  PIC_sendEOI(vector);
  return;
}

///////////
// Exception Handling
///////////

void pageFault_handler() {
  uint64_t cr3_copy, cr2_copy;
  __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_copy));
  __asm__ volatile("mov %%cr2, %0" : "=r"(cr2_copy));
  printk("Page fault\n");
  ERR_LOOP();
}

void exception_handler(uint32_t vector) {
  switch (vector) {
  case 0x00:
    printk("Divide-by-zero error\n");
    ERR_LOOP();
  case 0x01:
    printk("Debug exception\n");
    ERR_LOOP();
  case 0x02:
    printk("Non-maskable interrupt (NMI)\n");
    ERR_LOOP();
  case 0x03:
    printk("Breakpoint\n");
  case 0x04:
    debugk("Overflow\n");
  case 0x05:
    debugk("Bound range exceeded\n");
  case 0x06:
    printk("Invalid opcode\n");
    ERR_LOOP();
  case 0x07:
    printk("Device not available (FPU)\n");
    ERR_LOOP();
  case FAULT_DOUBLE:
    // NOTE: Unique Stack
    printk("Double fault\n");
    ERR_LOOP();
    goto exit_err_loop;
  case 0x0A:
    printk("Invalid TSS\n");
    ERR_LOOP();
  case 0x0B:
    printk("Segment not present\n");
    ERR_LOOP();
  case 0x0C:
    printk("Stack segment fault\n");
    ERR_LOOP();
  case FAULT_GENERAL_PROTECTION:
    // NOTE: Unique Stack
    printk("General protection fault\n");
    ERR_LOOP();
    goto exit_err_loop;
  case FAULT_PAGE:
    // NOTE: Unique Stack
    pageFault_handler();
    ERR_LOOP();
    goto exit_err_loop;
  case 0x10:
    printk("x87 FPU floating-point error\n");
    ERR_LOOP();
  case 0x11:
    printk("Alignment check\n");
    ERR_LOOP();
  case 0x12:
    printk("Machine check\n");
    ERR_LOOP();
  case 0x13:
    printk("SIMD floating-point exception\n");
    ERR_LOOP();
  default:
    printk("CPU exceptions %d\n", vector);
    ERR_LOOP();
  }
exit_err_loop:
  return;
}
