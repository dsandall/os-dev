#include "tester.h"
#include "book.h"
#include "printer.h"
#include "virtpage_alloc.h"
#include <stdint.h>

static inline void write_fullpage(void *p, uint64_t pagenum, uint64_t magic) {
  for (uint64_t *c = (uint64_t *)p; (c) < (uint64_t *)(p + PAGE_SIZE); c++) {
    *c = HASH(pagenum, magic, (uint64_t)c);
  }
};

static inline void check_fullpage(void *p, uint64_t pagenum, uint64_t magic) {
  for (uint64_t *c = (uint64_t *)p; c < (uint64_t *)(p + PAGE_SIZE); c++) {
    ASSERT(*c == (uint64_t)HASH(pagenum, magic, (uint64_t)c));
  }
};

bool generic_page_tester(uint64_t *static_array, uint64_t static_len,
                         uint64_t (*generic_alloc)(),
                         bool (*generic_free)(uint64_t)) {
  // stress test by allocating all pages, writing something unique to the full
  // page, and verifying the full page

  uint64_t pagenum = 0;
  const uint64_t magic = 7;

  // allocate as many pages as you can, record all pointers, write magic to full
  // page
  while ((static_array[pagenum] = generic_alloc()) != (uint64_t)NULL) {
    // write to the full page
    uint64_t p = static_array[pagenum];
    write_fullpage((void *)p, pagenum, magic);

    // and do the next, while checking bounds
    if (pagenum++ >= static_len) {
      // you cannot fit any more pointers in your static array
      tracek("cannot fit any more pointers in your static array. this could be "
             "intended, or indicative of a failure. proceeding with existing "
             "allocated "
             "pointers\n");
      pagenum--; // undoing the straw that broke the camel's back
      break;
    };
  };

  tracek("successfully allocated and wrote magic to %lu pages\n", pagenum);

  // verify each number in each page
  for (uint64_t i = 0; i < pagenum; i++) {
    uint64_t p = static_array[i];
    check_fullpage((void *)p, i, magic);
    ASSERT(i < static_len);
  }

  tracek("%lu pages were verified\n", pagenum);

  // free each page
  for (uint64_t i = 0; i < pagenum; i++) {
    ASSERT(i < static_len);
    uint64_t p = static_array[i];
    bool ret = generic_free(p);
    ASSERT(ret);
  }

  tracek("%lu pages were freed\n", pagenum);

  return true;
}

////////////////////////////////////////
//// PHYSICAL_ALLOCATOR_STRESSTEST
////////////////////////////////////////

// #define PHYSICAL_ALLOCATOR_STRESSTEST
#ifdef PHYSICAL_ALLOCATOR_STRESSTEST
#define TEST_PAGES 70000
phys_addr allpages[TEST_PAGES]; // just a bit over 65035
#endif

void testPageAllocator() {
#ifndef PHYSICAL_ALLOCATOR_STRESSTEST
  tracek("PHYSICAL_ALLOCATOR_STRESSTEST is not enabled\n");
#else
  // allocate and free a few pages , print the addresses
  // ensure that physical pages are reused when freed
  tracek("allocating and freeing (with printed addresses)...\n");
  for (int i = 0; i < 3; i++) {
    phys_addr p1, p2;
    p1 = MMU_pf_alloc();
    p2 = MMU_pf_alloc();
    tracek("alloc'd %lx\n", p1);
    tracek("alloc'd %lx\n", p2);

    MMU_pf_free(p1);
    tracek("free'd %lx\n", p1);

    // WARN: we leak a lil memory here
  }

  tracek("PHYSICAL_ALLOCATOR_STRESSTEST is enabled\n");
  tracek("allocating all pages, please wait...\n");
  generic_page_tester(allpages, TEST_PAGES, unsafe_MMU_pf_alloc, MMU_pf_free);
  generic_page_tester(allpages, TEST_PAGES, unsafe_MMU_pf_alloc, MMU_pf_free);
  generic_page_tester(allpages, TEST_PAGES, unsafe_MMU_pf_alloc, MMU_pf_free);
#endif
}

////////////////////////////////////////
//// VIRT_ALLOCATOR_STRESSTEST
////////////////////////////////////////

// #define VIRT_ALLOCATOR_STRESSTEST
#ifdef VIRT_ALLOCATOR_STRESSTEST
// WARN: this should trigger a special fault in the page table, if you allocate
// more virt pages than phys memory during a test
#define NUM_TEST_VPAGES 60000
uint64_t ptrs[NUM_TEST_VPAGES];
static uint64_t alloc_test_wrapper() { return MMU_alloc_page().raw; };
static bool free_test_wrapper(uint64_t addr) {
  return MMU_free_page((virt_addr_t)addr);
};
#endif

void testVirtPageAlloc() {
#ifndef VIRT_ALLOCATOR_STRESSTEST
  tracek("VIRT_ALLOCATOR_STRESSTEST is not enabled\n");
#else
  tracek("VIRT_ALLOCATOR_STRESSTEST is enabled\n");
  tracek("allocating all pages, please wait...\n");
  generic_page_tester(ptrs, NUM_TEST_VPAGES, alloc_test_wrapper,
                      free_test_wrapper);
#endif
}

////////////////////////////////////////
//// KMALLOC_STRESSTEST
////////////////////////////////////////
// #define KMALLOC_STRESSTEST
#ifdef KMALLOC_STRESSTEST
#include "kmalloc.h"
#endif

void testKmalloc() {

#ifndef KMALLOC_STRESSTEST

  tracek("KMALLOC_STRESSTEST is not enabled\n");
#else
  tracek("KMALLOC_STRESSTEST is enabled, starting...\n");
  // test kmalloc
  virt_addr_t allocated_pointer = kmalloc(69);
  uint64_t *someotherdata = (uint64_t *)(allocated_pointer.raw);
  *someotherdata = 0xBEEF;
  if (*someotherdata != (uint64_t)0xBEEF) {
    debugk("kmalloc not working\n");
    ERR_LOOP();
  }
  kfree(allocated_pointer);

  // test many more times
  const int num_tests = 12;
  virt_addr_t ptrs[num_tests];

#define ALLOC_SIZE i * 27 + 1
#define ALLOC_DATA (i * 65) % 18
  // test kmalloc
  for (int i = 0; i < num_tests; i++) {
    ptrs[i] = kmalloc(ALLOC_SIZE);
  }
  for (int i = 0; i < num_tests; i++) {
    uint64_t *dat = (uint64_t *)(ptrs[i].raw);
    *dat = ALLOC_DATA;
  }
  for (int i = 0; i < num_tests; i++) {
    ASSERT(*(uint64_t *)(ptrs[i].raw) == ALLOC_DATA);
  }
  for (int i = 0; i < num_tests; i++) {
    kfree(ptrs[i]);
  }

#define ALLOC_SIZE_2 i * 2048 + 1
#define ALLOC_DATA_2 (i * 65) % 18 + z
  // test large allocations
  for (int i = 0; i < num_tests; i++) {
    ptrs[i] = kmalloc(ALLOC_SIZE);
  }
  for (int i = 0; i < num_tests; i++) {
    uint64_t *dat = (uint64_t *)(ptrs[i].raw);

    for (int z = 0; z < (ALLOC_SIZE / sizeof(uint64_t)); z++) {
      tracek("i, z: %d %d\n", i, z);
      if (i == 7 && z == 13310)
        breakpoint();

      dat[z] = ALLOC_DATA;
    }
  }
  for (int i = 0; i < num_tests; i++) {
    for (int z = 0; z < (ALLOC_SIZE / sizeof(uint64_t)); z++) {
      ASSERT(*(uint64_t *)(ptrs[i].raw) == ALLOC_DATA);
    }
  }
  for (int i = 0; i < num_tests; i++) {
    kfree(ptrs[i]);
  }
#endif
}
