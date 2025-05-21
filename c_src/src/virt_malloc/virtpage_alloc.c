#include "virtpage_alloc.h"
#include "book.h"
#include "freestanding.h"
#include "rejigger_paging.h"
#include <stdint.h>
#include <string.h>

static void makePresentHelper(page_table_entry_t *pte) {
  phys_addr newentry = (phys_addr)MMU_pf_alloc();
  pte->present = 1;
  pte->rw = 1;
  pte->p_addr4k = (newentry >> 12);
};

static virt_addr_t findFreeHeapAddress() {
  // start at the current top level page table
  // WARN: assumes that the current page table is the kernel page table, or at
  // least has access to the kernel heap vaddrs
  page_table_entry_t *pml4;
  __asm__ volatile("mov %%cr3, %0" : "=r"(pml4));

  // walk along, starting from beginning of the kheap vaddrs
  virt_addr_t start = {.raw = VADDR_BOUND_RESERVED_KERNEL};

  // Walk PML4
  debugk("start addr is %lu \n", start.raw);
  debugk("pml4 index is %d\n", start.pml4_idx);
  page_table_entry_t *pml4e = &pml4[start.pml4_idx];
  if (!(pml4e->present)) {
    // then make it present
    debugk("allocating new l3 pagetable from freelist\n");
    makePresentHelper(pml4e);
  }

  // Walk PDPT
  page_table_entry_t *pdpte = &walk_pointer(pml4e)[start.pdpt_idx];
  if (!(pdpte->present)) {
    // then make it present
    debugk("allocating new l2 pagetable from freelist\n");
    makePresentHelper(pdpte);
  }
  if (pdpte->pse_or_pat) {
    debugk("is 1gb page, cannot allocate memory here\n");
    ERR_LOOP();
  }

  // Walk PD
  page_table_entry_t *pde = &walk_pointer(pdpte)[start.pd_idx];
  if (!(pde->present)) {
    // then make it present
    debugk("allocating new l1 pagetable from freelist\n");
    makePresentHelper(pde);
  }
  if (pde->pse_or_pat) {
    // 2 MiB page
    debugk("is 2mib page, cannot allocate memory here\n");
    ERR_LOOP();
  }

  // walk PT
  page_table_entry_t *pte = &walk_pointer(pde)[start.pt_idx];
  if (!(pte->present)) {
    // then mark it as ready to be allocated
    debugk("marking PTE as demanded\n");
    pte->raw = (uint64_t)0;
    pte->demanded = 1;
  } else {
    debugk("fix me, you only have one vaddr available\n");
    ERR_LOOP();
  }

  return start;
};

void *MMU_alloc_page() {
  // allocates a page, returns virt pointer to that page
  // - find free virt addr (page aligned) (can check page tables to do this)
  // - create new entry in page table (mark as "demanded", but don't alloc ppage
  // yet)
  // - (later,) page fault handler calls phys page malloc and completes last
  // level page table before returning control to sender
  //

  // find free virt addr
  return (void *)findFreeHeapAddress().raw;
};

void MMU_free_page(void *) {
  // frees page
  // - access page tables to map vaddr to p page
  // - free the associated physical page
  // - remove vaddr entry from page tables
  // - no need to free the vaddr, consider it dead
  // TODO:
};

///////////////////
/// pluralized funcs
///////////////////
void *MMU_alloc_pages(int num) {
  void *vaddr_array[num];
  for (int i = 0; i < num; i++) {
    vaddr_array[i] =
        MMU_alloc_page(); // WARN: assumes that vaddr is a list of pointers

    if (vaddr_array[i] == NULL) {
      ERR_LOOP();
    }
  }
  return vaddr_array; // TODO: malloc this? statically allocate in caller?
};

void MMU_free_pages(void *vaddr, int num) {
  for (int i = 0; i < num; i++) {
    MMU_free_page(vaddr++); // WARN: assumes that vaddr is a list of pointers
  }
};
