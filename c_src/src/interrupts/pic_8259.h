#ifndef PIC_H
#define PIC_H

#include "freestanding.h"

void PIC_set_mask(uint8_t PICline);
void PIC_clear_mask(uint8_t PICline);
uint16_t PIC_get_mask(void);
void do_PIC(void);

ISR_void PIC_common_handler(uint32_t vector);

#endif
