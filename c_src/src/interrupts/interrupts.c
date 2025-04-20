#include "interrupts.h"
#include "freestanding.h"
#include "macro_magic.h"
#include "pic_8259.h"
#include "printlib.h"
#include <stddef.h>
#include <stdint.h>

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

///////////////////////////////////////
/// update interrupt_descriptor_table register in CPU
///
Interrupt_CGD_t interrupt_descriptor_table[IDT_size];
///////////////////////////////////////

// load IDTR struct into the IDT Register using "lidt"
// This function works in 32 and 64bit mode
static inline void lidt(void *base, uint16_t size) {
  struct {
    uint16_t length;
    void *base;
  } __attribute__((packed)) IDTR = {size, base};

  asm("lidt %0" : : "m"(IDTR)); // let the compiler choose an addressing mode
}

void init_IDT(void) {
  for (int i = 0; i < IDT_size; i++) {
    uint64_t addr = (uint64_t)isr_wrappers[i];
    interrupt_descriptor_table[i] = (Interrupt_CGD_t){
        .add_1 = addr,
        .add_2 = addr >> 16,
        .add_3 = addr >> 32,
        .present = true,
        .DPL = X86_DPL_RING0,
        .gate_type = X86_GATETYPE_HWINT,
        .GDT_segment = X86_GDT_SEGMENT,
    };
  }

  lidt(&interrupt_descriptor_table,
       (uint16_t)(sizeof(Interrupt_CGD_t) * IDT_size) - 1);
};

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
