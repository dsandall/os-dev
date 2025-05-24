#ifndef REJIGGER_PAGING_H
#define REJIGGER_PAGING_H
#include "freestanding.h"
#include <stdint.h>
#define VADDR_BOUND_ID_MAP 0x40000000
#define VADDR_BOUND_RESERVED_USER 0x0000800000000000
#define VADDR_BOUND_RESERVED_DEADZONE 0xFFFF800000000000
#define VADDR_BOUND_RESERVED_ID_MAP 0xFFFF800100000000
#define VADDR_BOUND_RESERVED_KERNEL 0xFFFFFE0000000000
#define VADDR_BOUND_KHEAP 0xFFFFFF0000000000
// #define VADDR_BOUND_KSTACKS 0xFFFFFFFFFFFFFFFF
//
typedef uint64_t phys_addr;
#define OFFSET_1G 30
#define OFFSET_2M 21
#define OFFSET_4K 12
#define PTE_MAGIC 0x96

typedef union {
  uint64_t raw;
  void *point;
  struct {
    uint64_t offset_4k : OFFSET_4K;   // bits 0–11
    uint64_t pt_idx : 9;              // bits 12–20
    uint64_t pd_idx : 9;              // bits 21–29
    uint64_t pdpt_idx : 9;            // bits 30–38
    uint64_t pml4_idx : 9;            // bits 39–47
    uint64_t canonical_sign_ext : 16; // bits 48–63 (must match bit 47)
  };
  struct {
    uint64_t offset_2m : OFFSET_2M;
  };
  struct {
    uint64_t offset_1g : OFFSET_1G;
  };

} virt_addr_t;

typedef union {
  uint64_t raw;
  void *point;
  struct {
    uint64_t present : 1; // Bit 0
    uint64_t rw : 1;      // Read/write
    uint64_t : 5;
    uint64_t pse_or_pat : 1; // PAT on PTE, PSE on PDE
    uint64_t : 4;
    phys_addr p_addr4k : 40;
    uint64_t magic : 8;
    uint64_t demanded : 1; // bit 60
    uint64_t : 3;
  };
  struct {
    uint64_t : OFFSET_2M;
    phys_addr p_addr_2m : 31;
    uint64_t : 12;
  };
  struct {
    uint64_t : OFFSET_1G;
    phys_addr p_addr_1g : 22;
    uint64_t : 12;
  };
} page_table_entry_t;

typedef enum { FOUR_KAY = 1, TWO_MEG = 2, ONE_GIB = 3, MASTER = 4 } pte_level_t;

typedef struct {
  page_table_entry_t *pte;
  pte_level_t lvl;
} pte_and_level_t;

//////////////////////////////////////////////////////////////////

#define walk_pointer(p) ((page_table_entry_t *)(p->p_addr4k << 12))

pte_and_level_t walk_page_tables(virt_addr_t v, page_table_entry_t *master_l4);
void regenerate_page_tables();
phys_addr from_virtual(virt_addr_t v);
phys_addr from_entry(pte_and_level_t res, virt_addr_t v);

#endif // REJIGGER_PAGING_H
