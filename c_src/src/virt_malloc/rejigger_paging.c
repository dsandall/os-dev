#include "rejigger_paging.h"
#include "book.h"
#include "freestanding.h"
#include "multiboot.h"
#include "printer.h"
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
  // uint64_t *kernel_pml4 = create_page_table();

  // TODO: when redoing the identity map, unmap stuff that is outside the range
  // of actual physical memory! otherwise, it will not trigger a page fault, but
  // return gibberish

  // update the cpu reg that points to the master page table
  // (this should not change anything in theory, as the asm boot stub sets cr3
  // to this p4 table anyway)

  // __asm__ volatile("mov %0, %%cr3" ::"r"((uint64_t)p4_table) : "memory");

  // sanity check, make copy and reload it to the reg
  uint64_t cr3_copy;
  __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_copy));
  __asm__ volatile("mov %0, %%cr3" ::"r"((uint64_t)cr3_copy) : "memory");

  return;
}

//////////////////////////////////////////////////////////////////
static bool check_canonical_address(virt_addr_t v) {
  bool bit47 = (v.raw >> 47) & 1;
  uint16_t expected = bit47 ? 0xFFFF : 0x0000;
  return (expected == v.canonical_sign_ext);
}

bool where_is_vaddr(virt_addr_t v) {
  ASSERT(check_canonical_address(v));

  if (v.raw < VADDR_BOUND_ID_MAP) {
    printk("address is in the lower identity map\n");
    return true;
  } else if (v.raw < VADDR_BOUND_RESERVED_USER) {
    printk("address is somewhere in user space\n");
    return false;
  } else if (v.raw < VADDR_BOUND_RESERVED_DEADZONE) {
    printk("address is in the deadzone (non-canonical)\n");
    return false;
  } else if (v.raw < VADDR_BOUND_RESERVED_ID_MAP) {
    printk("address is in the reserved future ID map\n");
    return false;
  } else if (v.raw < VADDR_BOUND_RESERVED_KERNEL) {
    printk("address is in reserved kernel memory\n");
    return false;
  } else if (v.raw < VADDR_BOUND_KHEAP) {
    printk("address is in kernel heap\n");
    return true;
  } else {
    printk("address is (presumably) in some kernel stack\n");
    return true;
  }

  ERR_LOOP();
}

pte_and_level_t walk_page_tables(virt_addr_t v) {
  ASSERT(check_canonical_address(v));

  // WARN: might need to handle this differently in future
  page_table_entry_t *pml4;
  __asm__ volatile("mov %%cr3, %0" : "=r"(pml4));

  // Walk PML4
  page_table_entry_t *pml4e = &pml4[v.pml4_idx];
  ASSERT(pml4e->present);

  // Walk PDPT
  page_table_entry_t *pdpte = &walk_pointer(pml4e)[v.pdpt_idx];
  ASSERT(pdpte->present);

  if (pdpte->pse_or_pat) {
    // 1 GiB page (optional, not requested, but good to include)
    debugk("is 1gb page\n");
    return (pte_and_level_t){pdpte, JUAN_GEE};
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
  // WARN: check disabled for demand paging
  // if (!(pte.present))
  //  ERR_LOOP();

  debugk("is 4kib page\n");
  return (pte_and_level_t){pte, FOUR_KAY};
}

phys_addr from_entry(pte_and_level_t res, virt_addr_t v) {
  switch (res.lvl) {
  case FOUR_KAY:
    return (phys_addr)(res.pte->p_addr4k << OFFSET_4K) + v.offset_4k;
  case TWO_MEG:
    return (phys_addr)(res.pte->p_addr_2m << OFFSET_2M) + v.offset_2m;
  case JUAN_GEE:
    return (phys_addr)(res.pte->p_addr_1g << OFFSET_1G) + v.offset_1g;
  }
}

phys_addr from_virtual(virt_addr_t v) {

  ASSERT(check_canonical_address(v));

  pte_and_level_t res = walk_page_tables(v);

  switch (res.lvl) {
  case FOUR_KAY:
    if (res.pte->present)
      return from_entry(res, v);
    ERR_LOOP(); // you best hope its a demand page otherwise
  case TWO_MEG:
    return from_entry(res, v);
  case JUAN_GEE:
    return from_entry(res, v);
  }

  ERR_LOOP();
}

void testAddressTranslation() {
  uint32_t junk = 0;
  virt_addr_t v;
  v.raw = (uint64_t)&junk;

  // attempt translation for identity page by walking page table
  phys_addr p = from_virtual(v);
  debugk("regenerated page tablets\n");
  ASSERT(p == v.raw);
  debugk("passed identity page translation check\n");
}
