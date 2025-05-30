#include "rejigger_paging.h"
#include "book.h"
#include "freestanding.h"
#include "multiboot.h"
#include "printer.h"
#include "tester.h"
#include "virtpage_alloc.h"
#include <stdint.h>
// Static page tables (only for kernel initial boot) (only the first entry in p4
// is used for vaddr mapping)
//
// at boot we have:
// one entry in the p4, pointing to p3
// one entry in the p3, pointing to p2
// 512 2mb entries in the p2, for a total of 1 gib of ID mapped memory
// (0x0-0x1F_FFFF ..... 0x3fe0_0000-0x3FFF_FFFF) (30 bits, the first 31 bit addr
// is out of range, 0x4000_0000)

#define PT_ENTRIES 512
uint64_t kernel_p4_table[PT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
uint64_t kernel_p3_table[PT_ENTRIES]
    __attribute__((aligned(PAGE_SIZE))); // PDPT
uint64_t identity_p2_table[PT_ENTRIES]
    __attribute__((aligned(PAGE_SIZE))); // PD

//////////////////////////////////////////////////////////////////

void regenerate_page_tables() {

  // TODO: when redoing the identity map, unmap stuff that is outside the range
  // of actual physical memory! otherwise, it will not trigger a page fault, but
  // return gibberish

  page_table_entry_t *boot_master;
  __asm__ volatile("mov %%cr3, %0" : "=r"(boot_master));

  // PRIOR to calling KMALLOC:
  // alloc one page table for the heap
  page_table_entry_t *newmaster = (page_table_entry_t *)MMU_pf_alloc();

  ////// place existing id map in table
  newmaster[0] = boot_master[0]; // copy l4 entry from master

  page_table_entry_t *l3new = &walk_pointer(newmaster)[0];
  l3new[0] = walk_pointer(boot_master)[0]; // copy l3 entry from master

  l3new->magic = PTE_MAGIC; // only for l3 or lower

  // set as new cr3
  __asm__ volatile("mov %0, %%cr3" ::"r"(newmaster) : "memory");

  testVirtPageAlloc();

  return;
}

//////////////////////////////////////////////////////////////////
bool check_canonical_address(virt_addr_t v) {
  bool bit47 = (v.raw >> 47) & 1;
  uint16_t expected = bit47 ? 0xFFFF : 0x0000;
  return (expected == v.canonical_sign_ext);
}

pte_and_level_t walk_page_tables(virt_addr_t v, page_table_entry_t *master_l4) {

  BREAK_IF(v.raw == 0x70000001e);

  BREAK_IF(v.raw == 0x400000016);

  BREAK_IF(v.pdpt_idx == 16);

  ASSERT(check_canonical_address(v));

  // Walk PML4
  page_table_entry_t *pml4e = &master_l4[v.pml4_idx];
  ASSERT(pml4e->present);

  // Walk PDPT
  page_table_entry_t *pdpte = &walk_pointer(pml4e)[v.pdpt_idx];
  ASSERT(pdpte->present);

  if (pdpte->pse_or_pat) {
    // 1 GiB page (optional, not requested, but good to include)
    debugk("is 1gb page\n");
    return (pte_and_level_t){pdpte, ONE_GIB};
  }

  // Walk PD
  page_table_entry_t *pde = &walk_pointer(pdpte)[v.pd_idx];
  ASSERT(pde->present);

  if (pde->pse_or_pat) {
    // 2 MiB page
    debugk("is 2mib page\n");
    return (pte_and_level_t){pde, TWO_MEG};
  }

  // Walk PT
  page_table_entry_t *pte = &walk_pointer(pde)[v.pt_idx];

  // debugk("is 4kib page\n");
  return (pte_and_level_t){pte, FOUR_KAY};
}

phys_addr from_entry(pte_and_level_t res, virt_addr_t v) {
  switch (res.lvl) {
  case FOUR_KAY:
    return (phys_addr)(res.pte->p_addr4k << OFFSET_4K) + v.offset_4k;
  case TWO_MEG:
    return (phys_addr)(res.pte->p_addr_2m << OFFSET_2M) + v.offset_2m;
  case ONE_GIB:
    return (phys_addr)(res.pte->p_addr_1g << OFFSET_1G) + v.offset_1g;
  case MASTER:
    ERR_LOOP();
  }
}

static phys_addr from_virtual(virt_addr_t v) {

  ASSERT(check_canonical_address(v));

  page_table_entry_t *current_master;
  __asm__ volatile("mov %%cr3, %0" : "=r"(current_master));
  pte_and_level_t res = walk_page_tables(v, current_master);

  switch (res.lvl) {
  case FOUR_KAY:
    if (res.pte->present)
      return from_entry(res, v);
    ERR_LOOP();
  case TWO_MEG:
  case ONE_GIB:
  case MASTER:
    return from_entry(res, v);
  }
}

static void testAddressTranslation() {
  uint32_t junk = 0;
  virt_addr_t v;
  v.raw = (uint64_t)&junk;

  // attempt translation for identity page by walking page table
  phys_addr p = from_virtual(v);
  debugk("regenerated page tablets\n");
  ASSERT(p == v.raw);
  debugk("passed identity page translation check\n");
}
