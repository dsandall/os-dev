#ifndef REJIGGER_PAGING_H
#define REJIGGER_PAGING_H
#include "freestanding.h"
#include "paging.h"

#define PTE_MAGIC 0x96

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

typedef struct {
  page_table_entry_t *pte;
  enum { FOUR_KAY = 1, TWO_MEG = 2, ONE_GIB = 3, MASTER = 4 } lvl;
} pte_and_level_t;

//////////////////////////////////////////////////////////////////

#define walk_pointer(p) ((page_table_entry_t *)(p->p_addr4k << 12))

void regenerate_page_tables();

bool check_canonical_address(virt_addr_t v);

pte_and_level_t walk_page_tables(virt_addr_t v, page_table_entry_t *master_l4);
phys_addr from_virtual(virt_addr_t v);
phys_addr from_entry(pte_and_level_t res, virt_addr_t v);

#endif // REJIGGER_PAGING_H
