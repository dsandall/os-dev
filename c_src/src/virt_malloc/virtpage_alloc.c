#include "virtpage_alloc.h"
#include "book.h"
#include "freestanding.h"
#include "printer.h"
#include "rejigger_paging.h"
#include "tester.h"

static void makePresentHelper(pte_and_level_t pte) {
  phys_addr newentry = (phys_addr)MMU_pf_alloc();
  pte.pte->raw = 0;
  if (pte.lvl != MASTER)
    pte.pte->magic = PTE_MAGIC;
  pte.pte->present = 1;
  pte.pte->demanded = 0;
  pte.pte->rw = 1;
  pte.pte->p_addr4k = (newentry >> 12);
};

bool is_in_kheap(virt_addr_t v) {
  return (v.raw < VADDR_BOUND_KHEAP && v.raw >= VADDR_BOUND_RESERVED_KERNEL);
}

static pte_and_level_t alloc_helper(pte_and_level_t upper_entry,
                                    virt_addr_t v) {
  int idx;
  switch (upper_entry.lvl) {
  case MASTER:
    ERR_LOOP();
    break;
  case ONE_GIB:
    idx = v.pdpt_idx;
    break;
  case TWO_MEG:
    idx = v.pd_idx;
    break;
  case FOUR_KAY:
    idx = v.pt_idx;
    break;
  }

  // Walk
  pte_and_level_t ret = {.pte = &walk_pointer(upper_entry.pte)[idx],
                         upper_entry.lvl - 1};

  if (!(ret.pte->present)) {
    // then make it present
    debugk("allocating new l%d pagetable (l%d entry) from freelist\n", ret.lvl,
           upper_entry.lvl);
    tracek("associated vaddr is %p\n", v.point);
    makePresentHelper(ret);
  }

  ASSERT(ret.pte->magic == PTE_MAGIC);

  if (upper_entry.lvl - 1 >= TWO_MEG) {
    ASSERT(!ret.pte->pse_or_pat); // hugepage
  }

  return ret;
}

virt_addr_t heap_pointer = {.raw = VADDR_BOUND_RESERVED_KERNEL + PAGE_SIZE};

bool has_been_demanded(virt_addr_t v) { return (v.raw < heap_pointer.raw); }

virt_addr_t MMU_alloc_page() {
  // allocates a page, returns virt pointer to that page
  // - find free virt addr (page aligned) (can check page tables to do this)
  // - create new entry in page table (mark as "demanded", but don't alloc ppage
  // yet)
  // - (later,) page fault handler calls phys page malloc and completes last
  // level page table before returning control to sender

  virt_addr_t free_vp = heap_pointer;
  heap_pointer.raw += PAGE_SIZE;

  ASSERT(is_in_kheap(heap_pointer));
  /*
  debugk("demanding %lx \n", free_vp.raw);
  tracek("lvls:%d, %d, %d, %d\n", free_vp.pml4_idx, free_vp.pdpt_idx,
         free_vp.pd_idx, free_vp.pt_idx);
*/
  // WARN: assumes that the current page table is the kernel page table, or at
  // least has access to the kernel heap vaddrs

  // start at the current top level page table
  page_table_entry_t *pml4;
  __asm__ volatile("mov %%cr3, %0" : "=r"(pml4));

  page_table_entry_t *pml4e = &pml4[free_vp.pml4_idx];
  if (!(pml4e->present)) {
    // then make it present
    breakpoint();
    debugk("allocating new l3 pagetable (l4 entry) from freelist\n");
    makePresentHelper((pte_and_level_t){.pte = pml4e, .lvl = MASTER});
  }
  // ASSERT(pml4e->magic == PTE_MAGIC);

  // Walk PDPT
  pte_and_level_t pdpte =
      alloc_helper((pte_and_level_t){.pte = pml4e, ONE_GIB}, free_vp);

  // Walk PD
  pte_and_level_t pde = alloc_helper(pdpte, free_vp);

  // walk PT
  page_table_entry_t *pte = &walk_pointer(pde.pte)[free_vp.pt_idx];
  // tracek("pte is %p\n", pte);
  // tracek("pte.point is %p\n", pte->point);

  if ((pte->demanded && pte->magic == PTE_MAGIC)) {
    debugk("attempted to allocate page that was already demanded\n");
    ERR_LOOP();
  } else if ((pte->present) && pte->magic == PTE_MAGIC) {
    debugk("attempted to allocate page that already exists\n");
    ERR_LOOP();
  } else {
    // then mark it as ready to be allocated
    // debugk("marking PTE as demanded\n");
    pte->raw = (uint64_t)0;
    pte->demanded = 1;
    pte->magic = PTE_MAGIC;
  }

  return free_vp;
};

bool MMU_free_page(virt_addr_t v) {
  // frees page
  // - access page tables to map vaddr to p page
  // - free the associated physical page
  // - remove vaddr entry from page tables
  // - no need to free the vaddr, consider it dead

  page_table_entry_t *current_master;
  __asm__ volatile("mov %%cr3, %0" : "=r"(current_master));
  pte_and_level_t res = walk_page_tables(v, current_master);

  ASSERT(is_in_kheap(heap_pointer));
  ASSERT(res.lvl = FOUR_KAY);

  return MMU_pf_free(from_entry(res, v));
};

ISR_void pageFault_handler(uint32_t error) {
  virt_addr_t cr2_copy;
  page_table_entry_t *current_master;
  __asm__ volatile("mov %%cr3, %0" : "=r"(current_master));
  __asm__ volatile("mov %%cr2, %0" : "=r"(cr2_copy));

  // check for demand pages
  pte_and_level_t res = walk_page_tables((virt_addr_t)cr2_copy, current_master);

  if ((res.lvl == FOUR_KAY) && res.pte->demanded && !res.pte->present) {
    phys_addr newentry = (phys_addr)unsafe_MMU_pf_alloc();
    if (newentry != (phys_addr)NULL) {
      res.pte->p_addr4k = newentry >> 12;
      res.pte->present = 1;
      res.pte->demanded = 0;
      // tracek("pagefault handled gracefully (demand page)\n");
      return;
    } else {
      tracek("ran out of physical pages in page fault handler...\n");
    }
  }

  tracek("Page fault:\n\tfaulty addr:%p\n\tpage table in use:%p\n",
         cr2_copy.point, current_master);
  tracek("error is %d\n", error);
  ERR_LOOP();
}
