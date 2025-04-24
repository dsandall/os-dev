#include "freestanding.h"
#include <stdint.h>
#define STACK_SIZE 4096 // 4 KiB stack size

// Define the stack arrays (aligned to 4 KiB for TSS)
__attribute__((aligned(4096))) uint8_t stack_int3[STACK_SIZE];
__attribute__((aligned(4096))) uint8_t stack_int2[STACK_SIZE];
__attribute__((aligned(4096))) uint8_t stack_int1[STACK_SIZE];

__attribute__((aligned(4096))) uint8_t stack_kernel[STACK_SIZE];

struct __attribute__((packed)) TSS_t {
  uint32_t reserved0;
  uint64_t rsp0; // Kernel stack pointer (ring 0)
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved1;
  uint64_t ist1; // Interrupt stack for #GP
  uint64_t ist2; // Interrupt stack for #DF
  uint64_t ist3; // Interrupt stack for #PF
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iomap_base;
};

struct __attribute__((packed)) TSSDescriptor {
  unsigned lim_1 : 16;
  unsigned base_1 : 16;
  unsigned base_2 : 8;
  unsigned type : 4;
  unsigned : 1;
  unsigned DPL : 2;
  unsigned P : 1;
  unsigned lim_2 : 4;
  unsigned AVL : 1;
  unsigned : 1;
  unsigned G : 1;
  unsigned base_3 : 8;
  unsigned base_4 : 32;
  unsigned : 32;
};

// Example initialization of TSS
struct TSS_t tss = {
    .rsp0 = (uint64_t)(stack_kernel + STACK_SIZE), // Kernel stack (ring 0)
    .rsp1 = (uint64_t)(stack_kernel + STACK_SIZE), // Kernel stack (ring 0)
    .rsp2 = (uint64_t)(stack_kernel + STACK_SIZE), // Kernel stack (ring 0)
    .ist1 = (uint64_t)(stack_int1 + STACK_SIZE),   // #GP stack
    .ist2 = (uint64_t)(stack_int2 + STACK_SIZE),   // #DF stack
    .ist3 = (uint64_t)(stack_int3 + STACK_SIZE),   // #PF stack
    .reserved2 = 0,
    .reserved3 = 0,
    .iomap_base = sizeof(tss)};

//_attribute__((
//   aligned(16))) uint64_t gdt[5]; // 0 = null, 1 = code, 2 = data
//   (optional), 3
//                                  // = TSS (lo), 4 = TSS (hi)

static void gdt_install_tss(struct TSSDescriptor *tss_desc, int index);

void recreate_gdt() {
  // update GDT
  struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
  } gdt_ptr = {
      //.limit = sizeof(gdt) - 1,
      //.base = (uint64_t)&gdt[0],
  };
  __asm__ volatile("sgdt %0" : "=m"(gdt_ptr));

  __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));

  uint64_t (*gdt)[5] = (uint64_t (*)[5])gdt_ptr.base;

  (*gdt)[0] = 0; // Null descriptor
  (*gdt)[1] = (1ULL << 43) | (1ULL << 44) | (1ULL << 47) |
              (1ULL << 53); // .code segment
  (*gdt)[2] = 0;            //.data?
  (*gdt)[3] = 0;            // ts segment
  (*gdt)[4] = 0;            // ts segment

  __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));

  __asm__ volatile("sgdt %0" : "=m"(gdt_ptr));

  struct TSSDescriptor *tss_in_gdt = (struct TSSDescriptor *)&gdt[3];
  gdt_install_tss(tss_in_gdt, 3);

  __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));
}

static void gdt_install_tss(struct TSSDescriptor *tss_desc, int index) {
  // Generate new TSS descriptor
  uint64_t base = (uint64_t)&tss;
  uint32_t limit = sizeof(struct TSS_t) - 1;

  // Populate the descriptor fields using bitfields
  tss_desc->lim_1 = limit & 0xFFFF;
  tss_desc->base_1 = base & 0xFFFF;
  tss_desc->base_2 = (base >> 16) & 0xFF;
  tss_desc->type = 9; // Available 64-bit TSS
  tss_desc->DPL = 0;  // Descriptor Privilege Level (Ring 0)
  tss_desc->P = 1;    // Present bit
  tss_desc->lim_2 = (limit >> 16) & 0xF;
  tss_desc->AVL = 0; // Available (not used)
  tss_desc->G = 0;   // Granularity (no 4K granularity)
  tss_desc->base_3 = (base >> 24) & 0xFF;
  tss_desc->base_4 = (base >> 32) & 0xFFFFFFFF;

  // The remaining field is reserved and can be ignored in this case

  // Update the TSS
  uint16_t tss_selector = 8 * index;
  __asm__ volatile("ltr %0" : : "r"(tss_selector));
}
