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
