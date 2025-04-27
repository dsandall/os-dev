#include "book.h"

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

void initPageAllocator() {

};

phys_mem_region_t real_memory[100];
uint32_t num_regions;

void makePage(phys_mem_region_t available) {
  real_memory[num_regions++] = available;

  // free the page in the other structure
  // freePhysRegion();
};

void takePage(phys_mem_region_t not_available) {
  // mallocPhysRegion();
};
