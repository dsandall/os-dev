#include "freestanding.h"

typedef uint64_t phys_addr;
#define OFFSET_1G 30
#define OFFSET_2M 21
#define OFFSET_4K 12

typedef union {
  uint64_t raw;
  struct {
    uint64_t offset_4k : OFFSET_4K; // bits 0–11
    uint64_t pt_idx : 9;            // bits 12–20
    uint64_t pd_idx : 9;            // bits 21–29
    uint64_t pdpt_idx : 9;          // bits 30–38
    uint64_t pml4_idx : 9;          // bits 39–47
    uint64_t : 16;                  // bits 48–63
  };
  struct {
    uint64_t offset_2m : OFFSET_2M; // bits 0–11
  };
  struct {
    uint64_t offset_1g : OFFSET_1G; // bits 0–11
  };
} virt_addr_t;

void regenerate_page_tables();
phys_addr from_virtual(virt_addr_t v);
