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
// todo: exceptions have predefined table indexes

///////////////////////////////////////
/// The actual handler
///////////////////////////////////////
/*
struct interrupt_frame {
  uint64_t rip;   // "return to" address
  uint64_t cs;    // code segment executed from
  uint64_t flags; // interrupt vector #?
  uint64_t rsp; // stack pointer (assuming this is used for restoring state upon
                // return)
  uint64_t ss;  // stack segment
};

__attribute__((interrupt)) void
epic_interrupt_handler(struct interrupt_frame *frame) {

  uint8_t int_vector = frame->flags;

  // do something
  for (size_t i = 1; i <= 4; i++) {
    static int x;
    x++;
  }
  // printk("legitness\n");
}
*/

void asm_int_handler(uint16_t *ptr) {
  uint16_t val = *ptr; // val == 0x1234
  printk("%d\n", val);
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
