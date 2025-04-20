#include "freestanding.h"
#include "pic_8259.h"
#include "vga_textbox.h"

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

////////////////////////////////////
extern void isr_on_ps2_rx();
////////////////////////////////////

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
  case 0x08:
    printk("Double fault\n");
    ERR_LOOP();
  case 0x0A:
    printk("Invalid TSS\n");
    ERR_LOOP();
  case 0x0B:
    printk("Segment not present\n");
    ERR_LOOP();
  case 0x0C:
    printk("Stack segment fault\n");
    ERR_LOOP();
  case 0x0D:
    printk("General protection fault\n");
    ERR_LOOP();
  case 0x0E:
    printk("Page fault\n");
    ERR_LOOP();
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
  return;
}
