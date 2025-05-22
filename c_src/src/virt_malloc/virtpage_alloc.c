#include "virtpage_alloc.h"
#include "book.h"
#include "freestanding.h"
#include "rejigger_paging.h"
#include <stdint.h>
#include <string.h>
#define PTE_MAGIC 0x96
static void makePresentHelper(page_table_entry_t *pte) {
  phys_addr newentry = (phys_addr)MMU_pf_alloc();
  pte->magic = PTE_MAGIC;
  pte->present = 1;
  pte->demanded = 0;
  pte->rw = 1;
  pte->p_addr4k = (newentry >> 12);
};

// keeps track of the next free vaddr in the kernel heap
virt_addr_t heap_pointer = {.raw = VADDR_BOUND_RESERVED_KERNEL};

bool is_in_kheap(virt_addr_t v) {
  return (v.raw < VADDR_BOUND_KHEAP && v.raw >= VADDR_BOUND_RESERVED_KERNEL);
}

virt_addr_t MMU_alloc_page() {
  // allocates a page, returns virt pointer to that page
  // - find free virt addr (page aligned) (can check page tables to do this)
  // - create new entry in page table (mark as "demanded", but don't alloc ppage
  // yet)
  // - (later,) page fault handler calls phys page malloc and completes last
  // level page table before returning control to sender

  virt_addr_t free_vp = heap_pointer;

  // increment the vaddr tracker
  heap_pointer.raw += PAGE_SIZE;

  ASSERT(is_in_kheap(heap_pointer));

  // start at the current top level page table
  // WARN: assumes that the current page table is the kernel page table, or at
  // least has access to the kernel heap vaddrs
  page_table_entry_t *pml4;
  __asm__ volatile("mov %%cr3, %0" : "=r"(pml4));

  // Walk PML4
  debugk("start addr is %lx \n", free_vp.raw);
  debugk("pml4 index is %d\n", free_vp.pml4_idx);
  page_table_entry_t *pml4e = &pml4[free_vp.pml4_idx];
  if (!(pml4e->present)) {
    // then make it present
    debugk("allocating new l3 pagetable from freelist\n");
    makePresentHelper(pml4e);
  }
  ASSERT(pml4e->magic == PTE_MAGIC);

  // Walk PDPT
  page_table_entry_t *pdpte = &walk_pointer(pml4e)[free_vp.pdpt_idx];
  if (!(pdpte->present)) {
    // then make it present
    debugk("allocating new l2 pagetable from freelist\n");
    makePresentHelper(pdpte);
  }
  ASSERT(pdpte->magic == PTE_MAGIC);
  // 1 GiB page
  ASSERT(!pdpte->pse_or_pat);

  // Walk PD
  page_table_entry_t *pde = &walk_pointer(pdpte)[free_vp.pd_idx];
  tracek("pde is %p\n", pde);
  static page_table_entry_t *prev = NULL;
  if (prev == NULL) {
    prev = pde;
  } else if (prev != pde) {
    tracek("CHANGING OF GAURDS %p\n was %p\n", pde, prev);
    breakpoint();
    prev = pde;
  }
  if (!(pde->present)) {
    // then make it present
    debugk("allocating new l1 pagetable from freelist\n");
    makePresentHelper(pde);
  }
  ASSERT(pde->magic == PTE_MAGIC);
  // 2 MiB page
  ASSERT(!pde->pse_or_pat);

  // walk PT
  page_table_entry_t *pte = &walk_pointer(pde)[free_vp.pt_idx];
  tracek("pte is %p\n", pte);
  tracek("pte.point is %p\n", pte->point);

  static int tries = 0;
  if ((pte->demanded && pte->magic == PTE_MAGIC)) {
    debugk("attempted to allocate page that was already demanded\n");
    ERR_LOOP();
  } else if ((pte->present) && pte->magic == PTE_MAGIC) {
    debugk("attempted to allocate page that already exists\n");
    ERR_LOOP();

    tries++;
    debugk("took %d tries\n", tries);
    breakpoint();
    // WARN:
    // WARN:
    // WARN:
    // WARN:
    virt_addr_t v = MMU_alloc_page();
    ASSERT(pde->magic == PTE_MAGIC);
    // WARN:
    // WARN:
    // WARN:
    // WARN:
  } else {
    // then mark it as ready to be allocated
    debugk("marking PTE as demanded\n");
    pte->raw = (uint64_t)0;
    pte->demanded = 1;
    pte->magic = PTE_MAGIC;
  }

  tries = 0;
  return free_vp;
};

void MMU_free_page(virt_addr_t v) {
  // frees page
  // - access page tables to map vaddr to p page
  // - free the associated physical page
  // - remove vaddr entry from page tables
  // - no need to free the vaddr, consider it dead

  pte_and_level_t res = walk_page_tables(v);

  ASSERT(is_in_kheap(heap_pointer));
  ASSERT(res.lvl = FOUR_KAY);

  MMU_pf_free(from_entry(res, v));
};

void testVirtPageAlloc() {
  // test demand paging and vpage allocator
  uint64_t *somedata = (uint64_t *)MMU_alloc_page().raw;
  *somedata = 0xBEEF;
  ASSERT(*somedata == (uint64_t)0xBEEF);

  const int num = 800;
  virt_addr_t ptrs[num];
  for (int i = 0; i < num; i++) {
    tracek("%d\n", i);
    // if (i >= 512)
    //   breakpoint();

    ptrs[i] = MMU_alloc_page();

    tracek("ptrs[%d] = %lx\n", i, ptrs[i].raw);
    // try writing
    uint64_t *somedata = (uint64_t *)ptrs[i].raw;
    *somedata = i;
    ASSERT(*somedata == (uint64_t)i);
  }
  for (int i = 0; i < num; i++) {
    MMU_free_page(ptrs[i]);
    if (i != 0)
      tracek("ptrs[%d] = %lx\n", i - 1, ptrs[i - 1].raw);
  }
}
