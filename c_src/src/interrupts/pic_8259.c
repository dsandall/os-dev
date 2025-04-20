#include "freestanding.h"
#include "printer.h"
#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

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
// Interrupt Acking
///////////

#define PIC_EOI 0x20 /* End-of-interrupt command code */

void PIC_sendEOI(uint8_t irq) {
  if (irq >= 0x20) {
    int ext_int = irq - 0x20;
    if (ext_int >= 8) {
      outb(PIC2_COMMAND, PIC_EOI);
    } else {
      outb(PIC1_COMMAND, PIC_EOI);
    }
  }
}

///////////
// Initialization Remap
///////////

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

/*
arguments:
        offset1 - vector offset for master PIC
                vectors on the master become offset1..offset1+7
        offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2) {
  outb(PIC1_COMMAND,
       ICW1_INIT |
           ICW1_ICW4); // starts the initialization sequence (in cascade mode)
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
  io_wait();
  outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
  io_wait();
  outb(PIC1_DATA,
       4); // ICW3: tell Master PIC that there is a slave PIC a /t IRQ2 (0000
           // 0100)
  io_wait();
  outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
  io_wait();

  outb(PIC1_DATA,
       ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();
}

///////////
// Masking
///////////

/*
 * This register is a bitmap of the request lines going into the PIC. When a bit
 * is set, the PIC ignores the request and continues normal operation. Note that
 * setting the mask on a higher request line will not affect a lower line.
 * Masking IRQ2 will cause the Slave PIC to stop raising IRQs.
 */
void PIC_set_mask(uint8_t PICline) {
  uint16_t port;
  uint8_t value;

  if (PICline < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    PICline -= 8;
  }
  value = inb(port) | (1 << PICline);
  outb(port, value);
}

void PIC_clear_mask(uint8_t PICline) {
  uint16_t port;
  uint8_t value;

  if (PICline < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    PICline -= 8;
  }
  value = inb(port) & ~(1 << PICline);
  outb(port, value);
}

uint16_t PIC_get_mask(void) {
  return (uint16_t)(inb(PIC1_DATA) << 8 | inb(PIC2_DATA));
}

///////////////
// ISR and IRR
///////////////
/*https://wiki.osdev.org/8259_PIC#Real_Mode
 *The ISR tells us which interrupts are being serviced, meaning IRQs sent to the
 *CPU. The IRR tells us which interrupts have been raised. Based on the
 *interrupt mask (IMR), the PIC will send interrupts from the IRR to the CPU, at
 *which point they are marked in the ISR.

 Note that these functions will show bit 2 (0x0004) as on whenever any of the
 PIC2 bits are set, due to the chained nature of the PICs.
 (line 2 should not be used as an interrupt)
 * */
#define PIC_READ_IRR 0x0a /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR 0x0b /* OCW3 irq service next CMD read */

/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3) {
  /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
   * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
  outb(PIC1_COMMAND, ocw3);
  outb(PIC2_COMMAND, ocw3);
  return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void) { return __pic_get_irq_reg(PIC_READ_IRR); }

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void) { return __pic_get_irq_reg(PIC_READ_ISR); }

void do_PIC(void) {

  // remap to 0x20-0x2F
  PIC_remap(0x20, 0x28);

  printk("mask is %x\n", PIC_get_mask());
  // Unmask both PICs.
  outb(PIC1_DATA, 0);
  outb(PIC2_DATA, 0);
  printk("mask is %x\n", PIC_get_mask());

  printk("irr is %hx\n", pic_get_irr());
  printk("isr is %hx\n", pic_get_isr());
}
