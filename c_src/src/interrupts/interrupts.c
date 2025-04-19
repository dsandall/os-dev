#include "interrupts.h"
#include "freestanding.h"
#include "macro_magic.h"
#include "printlib.h"
#include <stddef.h>
#include <stdint.h>

// https://os.phil-opp.com/handling-exceptions/#the-interrupt-descriptor-table
typedef struct {
  unsigned add_1 : 16;
  unsigned GDT_segment : 16;
  unsigned interrupt_stack_table : 3;
  unsigned : 5;
  unsigned gate_type : 4;
  unsigned : 1;
  unsigned DPL : 2;
  unsigned present : 1;
  unsigned add_2 : 16;
  unsigned add_3 : 32;
  unsigned : 32;
} __attribute__((packed)) Interrupt_CGD_t;

#define IDT_size 256
Interrupt_CGD_t interrupt_descriptor_table[IDT_size];

///////////////////////////////////////
/// The actual handler
///////////////////////////////////////

extern void PIC_sendEOI(uint8_t);
void isr_common_handler(uint32_t vector);

void asm_int_handler(uint16_t *ptr) {
  uint16_t vector = *ptr;
  isr_common_handler(vector);
}

///////////////////////////////////////
/// update interrupt_descriptor_table register in CPU
///////////////////////////////////////

// load IDTR struct into the IDT Register using "lidt"
static inline void lidt(void *base, uint16_t size) {
  // This function works in 32 and 64bit mode
  struct {
    uint16_t length;
    void *base;
  } __attribute__((packed)) IDTR = {size, base};

  asm("lidt %0" : : "m"(IDTR)); // let the compiler choose an addressing mode
}

#define X86_GATETYPE_HWINT 0xE
#define X86_GATETYPE_TRAP 0xF
#define X86_DPL_RING0 0      // WARN: not sure abt this one
#define X86_GDT_SEGMENT 0x08 // WARN: not sure abt this one

//
/* Note from Dr. Bellardo: */
/*I strongly encourage you to use interrupt gates so interrupts are
 *automatically disabled while your ISR is running. However you are free to use
 *trap gates. Just make an explicit decision and understand how it impacts the
 *rest of your interrupt handling code
 */

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
////////////////////////////////////
////////////////////////////////////
////////////////////////////////////
////////////////////////////////////
////////////////////////////////////
#include "../ipc_channel/channel.h"

extern ipc_channel_t chan;
extern int wake_flags[]; // one per task
extern uint8_t PS2_RX_wrap();

void isr_on_ps2_rx() {
  uint8_t byte = PS2_RX_wrap();
  channel_send(&chan, byte);
  wake_flags[0] = 1; // wake task 0
}
////////////////////////////////////
////////////////////////////////////
////////////////////////////////////
////////////////////////////////////
////////////////////////////////////
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

void PIC_common_handler(uint32_t vector) {
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
    printk("COM1 (IRQ4)\n");
    ERR_LOOP();
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
  return;
}

void isr_common_handler(uint32_t vector) {

  if (vector < 0x20) {
    // exceptions
    exception_handler(vector);
    return;
  } else if (vector <= 0x2F) {
    // Hardware IRQs (0x20 - 0x2F)
    PIC_common_handler(vector);
    PIC_sendEOI(vector);
    return;
  } else {
    printk("Unknown CPU exception: 0x%x\n", vector);
    ERR_LOOP();
  }
}
