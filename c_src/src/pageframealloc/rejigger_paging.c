#include "rejigger_paging.h"
#include "book.h"
#include "multiboot.h"
#include <stdint.h>

// Static page tables (only for kernel initial boot) (only the first entry in p4
// is used for vaddr mapping)
//
// at boot we have:
// one entry in the p4, pointing to p3
// one entry in the p3, pointing to p2
// 512 entries in the p2, with valid entries:
//
//
#define L4_ENTRIES 512
#define L3_ENTRIES 512
#define L2_ENTRIES 512
#define L1_ENTRIES 512
__attribute__((aligned(4096))) uint64_t p4_table[L4_ENTRIES]; // PML4 top level
__attribute__((aligned(4096))) uint64_t p3_table[L3_ENTRIES]; // PDPT
__attribute__((aligned(4096))) uint64_t p2_table[L2_ENTRIES]; // PD
//////////////////////////////////////////////////////////////////

typedef union {
  uint64_t raw;
  struct {
    uint64_t present : 1; // Bit 0
    uint64_t rw : 1;      // Read/write
    uint64_t : 5;
    uint64_t pse_or_pat : 1; // PAT on PTE, PSE on PDE
    uint64_t : 4;
    phys_addr p_addr4k : 40;
    uint64_t : 12;
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
//////////////////////////////////////////////////////////////////
void regenerate_page_tables() {
  // uint64_t *kernel_pml4 = create_page_table();

  // update the cpu reg that points to the master page table
  // (this should not change anything in theory, as the asm boot stub sets cr3
  // to this p4 table anyway)
  __asm__ volatile("mov %0, %%cr3" ::"r"((uint64_t)p4_table) : "memory");

  // sanity check, make copy and reload it to the reg
  uint64_t cr3_copy;
  __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_copy));
  __asm__ volatile("mov %0, %%cr3" ::"r"((uint64_t)cr3_copy) : "memory");

  return;
}
//////////////////////////////////////////////////////////////////
#define walk(p) ((page_table_entry_t *)(p.p_addr4k << 12))
phys_addr from_virtual(virt_addr_t v) {

  // WARN: might need to handle this differently in future
  page_table_entry_t *pml4;
  __asm__ volatile("mov %%cr3, %0" : "=r"(pml4));

  // Walk PML4
  page_table_entry_t pml4e = pml4[v.pml4_idx];
  if (!(pml4e.present))
    ERR_LOOP();

  // Walk PDPT
  page_table_entry_t pdpte = walk(pml4e)[v.pdpt_idx];
  if (!(pdpte.present))
    ERR_LOOP();
  if (pdpte.pse_or_pat) {
    // 1 GiB page (optional, not requested, but good to include)
    printk("is 1gb page\n");
    return (phys_addr)(pdpte.p_addr_1g << OFFSET_1G) + v.offset_1g;
  }

  // Walk PD
  page_table_entry_t pde = walk(pdpte)[v.pd_idx];
  if (!(pde.present))
    ERR_LOOP();
  if (pde.pse_or_pat) {
    // 2 MiB page
    printk("is 2mib page\n");
    return (phys_addr)(pde.p_addr_2m << OFFSET_2M) + v.offset_2m;
  }

  // Walk PT
  page_table_entry_t pte = walk(pde)[v.pt_idx];
  if (!(pte.present))
    ERR_LOOP();

  printk("is 4kib page\n");
  return (phys_addr)(pte.p_addr4k << OFFSET_4K) + v.offset_4k;
}
