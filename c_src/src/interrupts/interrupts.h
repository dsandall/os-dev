// typedef void (*irq_handler_t)(int, int, void *);

// void IRQ_init(void);
// void IRQ_set_mask(int irq);
// void IRQ_clear_mask(int irq);
// int IRQ_get_mask(int IRQline);
// void IRQ_end_of_interrupt(int irq);
//
// void IRQ_set_handler(int irq, irq_handler_t handler, void *arg);
//
//
void init_IDT(void);

///////////////////////////////////////
/// The IDT Spec:
///////////////////////////////////////

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
