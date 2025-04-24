#include "freestanding.h"
#include "interrupts.h"
#include <stdint.h>

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

struct __attribute__((aligned(16))) __attribute__((packed)) TSSDescriptor {
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

// Define the stack arrays (aligned to 4 KiB for TSS)
__attribute__((aligned(4096))) uint8_t stack_int3[STACK_SIZE];
__attribute__((aligned(4096))) uint8_t stack_int2[STACK_SIZE];
__attribute__((aligned(4096))) uint8_t stack_int1[STACK_SIZE];

__attribute__((aligned(4096))) uint8_t stack_kernel[STACK_SIZE];

// the TSS
struct __attribute__((aligned(16))) TSS_t tss = {
    .rsp0 = (uint64_t)(stack_kernel + STACK_SIZE), // Kernel stack (ring 0)
    .rsp1 = (uint64_t)(stack_kernel + STACK_SIZE), // Kernel stack (ring 0)
    .rsp2 = (uint64_t)(stack_kernel + STACK_SIZE), // Kernel stack (ring 0)
    .ist1 = (uint64_t)(stack_int1 + STACK_SIZE),   // #GP stack
    .ist2 = (uint64_t)(stack_int2 + STACK_SIZE),   // #DF stack
    .ist3 = (uint64_t)(stack_int3 + STACK_SIZE),   // #PF stack
    .reserved2 = 0,
    .reserved3 = 0,
    .iomap_base = sizeof(tss)};

// the new gdt
__attribute__((aligned(16))) uint64_t gdt[4];

static void gdt_install_tss(struct TSSDescriptor *tss_desc);
static const int tss_index = 2;

void recreate_gdt() {

  (gdt)[0] = 0; // Null descriptor
  (gdt)[1] = (1ULL << 43) | (1ULL << 44) | (1ULL << 47) |
             (1ULL << 53); // .code segment
  (gdt)[2] = 0;            // ts segment
  (gdt)[3] = 0;            // ts segment

  struct TSSDescriptor *tss_in_gdt = (struct TSSDescriptor *)&gdt[tss_index];
  gdt_install_tss(tss_in_gdt);

  // update GDT
  struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
  } gdt_ptr = {
      .limit = sizeof(gdt) - 1,
      .base = (uint64_t)&gdt[0],
  };

  __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));
  __asm__ volatile("sgdt %0" : "=m"(gdt_ptr));

  // Update the TSS
  uint16_t tss_selector = 8 * tss_index;
  __asm__ volatile("ltr %0" : : "r"(tss_selector));

  uint16_t tr;
  __asm__ volatile("str %0" : "=r"(tr));

  if (tr != tss_selector) {
    ERR_LOOP();
  }
}

static void gdt_install_tss(struct TSSDescriptor *tss_desc) {
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
}
