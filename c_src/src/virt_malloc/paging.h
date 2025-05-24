#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#define PAGE_SIZE 4096

typedef uint64_t phys_addr;

#define OFFSET_1G 30
#define OFFSET_2M 21
#define OFFSET_4K 12

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

#endif
