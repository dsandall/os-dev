#include "book.h"
#include "freestanding.h"
#include "multiboot.h"
#include "printer.h"
#include "rejigger_paging.h"
#include <stdint.h>

#include "tester.h"

//////////////////////
//////////////////////
//////////////////////
//////////////////////
// For 64-bit address spaces only (e.g., what we are using in class with orders
// more virtual addresses than plausible physical memory), map the page frames
// at their associated phyiscal address in the virtual address space. This is
// similar to creating an identity mapping in the lower portion of the address
// space, leaving the remainder of the space for virtual addresses.
////////
/// There will be kernel THREADS, which are different than the main stack i
/// guess
/// /////
/// Kernel heap has some single region in virtual address space (maps to physic
/// pages anywhere its available)
/// /////
/// same for kernel stacks (one for each thread, which operates in virtual addr
/// space (unless it needs to acces phys addrs, in that case it just uses the
/// identity section at bottom of virtual memory))
/// /////
///

typedef struct recursive_page {
  struct recursive_page *next;
} page_t;

static page_t *free_list = NULL;

int makePhysPage(phys_mem_region_t available) {

  uintptr_t initial_base =
      (available.base + PAGE_SIZE - 1) &
      ~(PAGE_SIZE - 1); // align up (we will lose a little bit of memory if not
                        // page aligned, but we cant have partial pages)

  uintptr_t base = initial_base;
  // we are not going to use the first page of memory, because it's address is
  // NULL. Too much hassle for 4096 bytes

  if (base == 0) {
    base = PAGE_SIZE;
  }

  uintptr_t given_end = available.base + available.size;
  uintptr_t new_size = given_end - base;
  uintptr_t end = base + new_size;

  int pages_allocated = 0;
  while (base + PAGE_SIZE <= end) {
    // WARN: I add offset to the physical pointer here.
    // is phys addr, kernel still using vaddr identity mapping
    page_t *page =
        (page_t *)(base + MULTIBOOT_VADDR_OFFSET); // create pointer to the
                                                   // first occupied page ()
                                                   // for 256M
    if (page == (void *)0x10000000) {
      break;
      // WARN: this is really stupid but i am quite frustrated at the moment
      // this fixes the error. it also limits the available ram
    }

    page->next = free_list; // push existing onto the new head

    ASSERT(!(page->next == NULL && free_list != NULL));

    free_list = page; // set the new head

    base += PAGE_SIZE; // and keep going
    pages_allocated++;
  }

  return pages_allocated;
}

phys_addr unsafe_MMU_pf_alloc(void) {
  void *next_free_page = (void *)free_list;

  if (next_free_page == NULL) {
    return (phys_addr)NULL;
  }

  free_list = free_list->next;

  return (phys_addr)next_free_page;
}

phys_addr MMU_pf_alloc(void) {
  phys_addr p = unsafe_MMU_pf_alloc();

  ASSERT(p > 0x400000000);

  ASSERT(p != (uint64_t)NULL);

  return p;
};

bool MMU_pf_free(phys_addr pf) {

  ASSERT(pf >= (phys_addr)MULTIBOOT_VADDR_OFFSET);

  /*
  phys_mem_region_t returned_page = {((uint64_t)pf) - VIRT_MEM_OFFSET,
                                     PAGE_SIZE};
  if (makePage(returned_page) != 1) {
   ERR_LOOP();
  };
  */

  page_t *list = free_list;
  free_list = (page_t *)pf;
  free_list->next = list;

  return true;
};
