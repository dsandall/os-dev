#include "freestanding.h"
#include "interrupts.h"
#include "macro_magic.h"
///////////////////////////////////////
/// The IDT Spec:
///////////////////////////////////////

// https://os.phil-opp.com/handling-exceptions/#the-interrupt-descriptor-table

#define IDT_size 256
#define X86_GATETYPE_HWINT 0xE
#define X86_GATETYPE_TRAP 0xF
#define X86_DPL_RING0 0          // WARN: not sure abt this one
#define X86_GDT_SEGMENT (1 << 3) // WARN: not sure abt this one

//
/* Note from Dr. Bellardo: */
/*I strongly encourage you to use interrupt gates so interrupts are
 *automatically disabled while your ISR is running. However you are free to use
 *trap gates. Just make an explicit decision and understand how it impacts the
 *rest of your interrupt handling code
 */

typedef struct {
  unsigned add_1 : 16;
  unsigned GDT_segment : 16;
  unsigned IST_index : 3;
  unsigned : 5;
  unsigned gate_type : 4;
  unsigned : 1;
  unsigned DPL : 2;
  unsigned present : 1;
  unsigned add_2 : 16;
  unsigned add_3 : 32;
  unsigned : 32;
} __attribute__((packed)) Interrupt_CGD_t;

///////////////////////////////////////
/// update interrupt_descriptor_table register in CPU
///
static Interrupt_CGD_t interrupt_descriptor_table[IDT_size];
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
    interrupt_descriptor_table[i] =
        (Interrupt_CGD_t){.add_1 = addr,
                          .add_2 = addr >> 16,
                          .add_3 = addr >> 32,
                          .present = true,
                          .DPL = X86_DPL_RING0,
                          .gate_type = X86_GATETYPE_HWINT,
                          .GDT_segment = X86_GDT_SEGMENT,
                          .IST_index = 0};
  }

  // assign certain faults their own stack
  interrupt_descriptor_table[FAULT_GENERAL_PROTECTION].IST_index = 1;
  interrupt_descriptor_table[FAULT_DOUBLE].IST_index = 1;
  interrupt_descriptor_table[FAULT_PAGE].IST_index = 1;

  lidt(&interrupt_descriptor_table,
       (uint16_t)(sizeof(Interrupt_CGD_t) * IDT_size) - 1);
};
