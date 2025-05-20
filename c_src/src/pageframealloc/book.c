#include "book.h"
#include "freestanding.h"
#include "multiboot.h"
#include "printer.h"
#include <stdint.h>

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

int makePage(phys_mem_region_t available) {

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

    if (page->next == NULL && free_list != NULL) {
      ERR_LOOP();
    }

    free_list = page; // set the new head

    base += PAGE_SIZE; // and keep going
    pages_allocated++;
  }

  return pages_allocated;
}

void *MMU_pf_alloc(void) {
  void *next_free_page = (void *)free_list;
  if (next_free_page == NULL) {
    return NULL;
  }

  free_list = free_list->next;
  if (next_free_page == (void *)0xFFFFFFFFFFFFFFFF) {
    ERR_LOOP();
  }

  return next_free_page;
};

bool MMU_pf_free(void *pf) {

  if (pf < (void *)MULTIBOOT_VADDR_OFFSET) {
    ERR_LOOP();
    // should be a virt addr
  }

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

#undef PHYSICAL_ALLOCATOR_STRESSTEST
#ifdef PHYSICAL_ALLOCATOR_STRESSTEST
uint64_t allpages[70000]; // just a bit over 65035

void testPageAllocator_stresstest() {
  const int magic = 7;
  // stress test by allocating all pages, writing something unique to the full
  // page, and verifying the full page

  // allocate as many pages as you can, record all pointers, write magic to full
  // page

  int pagenum = 0;

  while ((allpages[pagenum] = (uint64_t)MMU_pf_alloc()) != NULL) {
    // write to the full page
    void *p = (void *)allpages[pagenum];
    for (uint64_t *c = p; ((void *)c) < (p + PAGE_SIZE); c++) {
      *c = pagenum + magic;
    }
    // and do the next
    pagenum++;
  };

  printk("successfully allocated and wrote magic to %d pages\n", pagenum);

  // verify each number in each page
  for (int i = 0; i < pagenum; i++) {
    void *p = (void *)allpages[i];
    for (uint64_t *c = p; ((void *)c) < (p + PAGE_SIZE); c++) {
      if (*c != (uint64_t)(i + magic)) {
        ERR_LOOP();
      }
    }
  }

  printk("%d pages were verified\n", pagenum);

  // free each page
  for (int i = 0; i < pagenum; i++) {
    void *p = (void *)allpages[i];
    if (MMU_pf_free(p) == false) {
      ERR_LOOP();
    };
  }

  printk("%d pages were freed\n", pagenum);
}
#endif

void testPageAllocator() {
  // allocate and free a few pages , print the addresses
  // ensure that physical pages are reused when freed
  printk("allocating and freeing (with printed addresses)...\n");
  for (int i = 0; i < 3; i++) {
    void *p1, *p2;
    p1 = MMU_pf_alloc();
    p2 = MMU_pf_alloc();
    printk("alloc'd %lu\n", (uint64_t)p1);
    printk("alloc'd %lu\n", (uint64_t)p2);

    MMU_pf_free(p1);
    printk("free'd %lu\n", (uint64_t)p1);

    // WARN: we leak a lil memory here
  }

#ifndef PHYSICAL_ALLOCATOR_STRESSTEST
  printk("stresstest is not enabled\n");
#else
  printk("stresstest is enabled\n");
  printk("allocating all pages, please wait...\n");
  testPageAllocator_stresstest();
  testPageAllocator_stresstest();
  testPageAllocator_stresstest();
#endif
}
