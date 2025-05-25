#include "interrupts.h"
#include "book.h"
#include "freestanding.h"
#include "pic_8259.h"
#include "printer.h"
#include "rejigger_paging.h"
#include "virtpage_alloc.h"
#include <stdint.h>

// https://wiki.osdev.org/8259_PIC#Programming_with_the_8259_PIC
// void interrupt_handler(void) { tracek("interrupting cow goes moooo\n"); }

///////////////////////////////////////
/// The actual handler
///////////////////////////////////////

ISR_void exception_handler(uint32_t vector, uint32_t error);

ISR_void asm_int_handler(uint16_t vector, uint32_t error) {
  if (vector < 0x20) {
    // exceptions
    exception_handler(vector, error);
    return;
  } else if (vector <= 0x2F) {
    // Hardware IRQs (0x20 - 0x2F)
    PIC_common_handler(vector);
    return;
  } else if (vector == 0x80) {
    tracek("yielding\n");
  } else {
    tracek("Unknown CPU exception: 0x%x\n", vector);
    ERR_LOOP();
  }
}

///////////
// Interrupt Handling
///////////

ISR_void isr_on_ps2_rx();
void PIC_sendEOI(uint8_t irq);
extern ISR_void isr_on_hw_serial();

ISR_void PIC_common_handler(uint32_t vector) {
  switch (vector) {
  case 0x20:
    // tracek("Timer interrupt (IRQ0)\n");
    break;
  case 0x21:
    // tracek("Keyboard interrupt (IRQ1)\n");
    isr_on_ps2_rx();
    break;
  case 0x22:
    tracek("Cascade (IRQ2, usually unused)\n");
    ERR_LOOP();
  case 0x23:
    tracek("COM2 (IRQ3)\n");
    ERR_LOOP();
  case 0x24:
    // tracek("COM1 (IRQ4)\n");
    isr_on_hw_serial();
    break;
  case 0x25:
    tracek("LPT2 / floppy (IRQ5)\n");
    ERR_LOOP();
  case 0x26:
    tracek("Floppy / sound card (IRQ6)\n");
    ERR_LOOP();
  case 0x27:
    tracek("LPT1 / spurious (IRQ7)\n");
    ERR_LOOP();
  case 0x28:
    tracek("RTC / CMOS (IRQ8)\n");
    ERR_LOOP();
  case 0x29:
    tracek("ACPI / legacy SCSI (IRQ9)\n");
    ERR_LOOP();
  case 0x2A:
    tracek("Unused / network / USB (IRQ10)\n");
    ERR_LOOP();
  case 0x2B:
    tracek("Unused / SCSI (IRQ11)\n");
    ERR_LOOP();
  case 0x2C:
    tracek("PS/2 mouse (IRQ12)\n");
    ERR_LOOP();
  case 0x2D:
    tracek("FPU / coprocessor / IRQ13\n");
    ERR_LOOP();
  case 0x2E:
    tracek("Primary ATA (IRQ14)\n");
    ERR_LOOP();
  case 0x2F:
    tracek("Secondary ATA (IRQ15)\n");
    ERR_LOOP();
  }
  PIC_sendEOI(vector);
  return;
}

///////////
// Exception Handling
///////////

ISR_void exception_handler(uint32_t vector, uint32_t error) {
  switch (vector) {
  case 0x00:
    tracek("Divide-by-zero error\n");
    ERR_LOOP();
  case 0x01:
    tracek("Debug exception\n");
    ERR_LOOP();
  case 0x02:
    tracek("Non-maskable interrupt (NMI)\n");
    ERR_LOOP();
  case 0x03:
    tracek("Breakpoint\n");
  case 0x04:
    debugk("Overflow\n");
  case 0x05:
    debugk("Bound range exceeded\n");
  case 0x06:
    tracek("Invalid opcode\n");
    ERR_LOOP();
  case 0x07:
    tracek("Device not available (FPU)\n");
    ERR_LOOP();
  case FAULT_DOUBLE:
    // NOTE: Unique Stack
    tracek("Double fault\n");
    ERR_LOOP();
    goto exit_err_loop;
  case 0x0A:
    tracek("Invalid TSS\n");
    ERR_LOOP();
  case 0x0B:
    tracek("Segment not present\n");
    ERR_LOOP();
  case 0x0C:
    tracek("Stack segment fault\n");
    ERR_LOOP();
  case FAULT_GENERAL_PROTECTION:
    // NOTE: Unique Stack
    tracek("General protection fault\n");
    tracek("error is %d\n", error);
    ERR_LOOP();
    goto exit_err_loop;
  case FAULT_PAGE:
    // NOTE: Unique Stack
    pageFault_handler(error);
    goto exit_err_loop;
  case 0x10:
    tracek("x87 FPU floating-point error\n");
    ERR_LOOP();
  case 0x11:
    tracek("Alignment check\n");
    ERR_LOOP();
  case 0x12:
    tracek("Machine check\n");
    ERR_LOOP();
  case 0x13:
    tracek("SIMD floating-point exception\n");
    ERR_LOOP();
  default:
    tracek("CPU exceptions %d\n", vector);
    ERR_LOOP();
  }
exit_err_loop:
  return;
}
