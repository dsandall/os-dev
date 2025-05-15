#include "freestanding.h"
void *MMU_alloc_page() {
  // allocates a page, returns virt pointer to that page
  // - find free virt addr (page aligned) (can check page tables to do this)
  // - create new entry in page table (mark as "alloc'd", but don't alloc ppage
  // yet)
  // - (later,) page fault handler calls phys page malloc and completes last
  // level page table before returning control to sender
  //

  // find free virt addr
  // TODO:
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
