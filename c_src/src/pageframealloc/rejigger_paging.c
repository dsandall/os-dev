#include "book.h"
#include "freestanding.h"
#define PAGE_PRESENT 0x1ULL
#define PAGE_RW 0x2ULL
#define PAGE_SIZE_FLAG 0x80ULL

#define L4_ENTRIES 512
#define L3_ENTRIES 512
#define L2_ENTRIES 512
#define L1_ENTRIES 512

// Static page tables (only for kernel initial boot) (only the first entry in p4
// is used for vaddr mapping)
__attribute__((aligned(4096))) uint64_t p4_table[L4_ENTRIES]; // PML4 top level
__attribute__((aligned(4096))) uint64_t p3_table[L3_ENTRIES]; // PDPT
__attribute__((aligned(4096))) uint64_t p2_table[L2_ENTRIES]; // PD

// test
//__attribute__((aligned(4096))) uint64_t pml4[1]; // PML4 top level
//__attribute__((aligned(
//    4096))) uint64_t pml3[L3_ENTRIES]; // PDPT // WARN: should be 2d array
//__attribute__((aligned(4096))) uint64_t pml2[L3_ENTRIES][L2_ENTRIES]; // PD
//__attribute__((
//    aligned(4096))) uint64_t pml1[L3_ENTRIES][L2_ENTRIES][L1_ENTRIES]; // PT

// uint64_t *create_page_table() {
//
//   // we can start by populating the first entry in the top level table as
//   // identity map
//   //
//   uint64_t i4 = 0;
//   pml4[i4] = (uint64_t)&pml3[i4] | PAGE_PRESENT | PAGE_RW;
//
//   // and recursively fill all other lower layers
//   for (int i3 = 0; i3 < L3_ENTRIES; i3++) {
//     // point each entry of the l3 table to the start of an l2 table
//     pml3[i3] = (uint64_t)&pml2[i3] | PAGE_PRESENT | PAGE_RW;
//
//     for (int i2 = 0; i2 < L2_ENTRIES; i2++) {
//       // point each entry of the l2 table to the start of an l1 table
//       pml2[i3][i2] = (uint64_t)&(pml1[i3][i2]) | PAGE_PRESENT | PAGE_RW;
//
//       for (int i1 = 0; i1 < L1_ENTRIES; i1++) {
//         // point each entry of the l1 table to the actual associated physical
//         // page
//         uint64_t phys_addr = 0 << 9;
//         phys_addr = (phys_addr | i3) << 9;
//         phys_addr = (phys_addr | i2) << 9;
//         phys_addr = (phys_addr | i1) << 9;
//         phys_addr = (phys_addr) << 12;
//         pml1[i3][i2][i1] = phys_addr;
//       }
//     }
//   }
//   return pml4;
// }

void regenerate_page_tables() {
  // uint64_t *kernel_pml4 = create_page_table();

  // update the cpu reg that points to the master page table
  // TODO: update me when switching to user mode, as each process should have
  // its own set of page tables

  __asm__ volatile("mov %0, %%cr3" ::"r"((uint64_t)p4_table) : "memory");

  uint64_t cr3_copy;
  __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_copy));
  __asm__ volatile("mov %0, %%cr3" ::"r"((uint64_t)cr3_copy) : "memory");

  return;
}
