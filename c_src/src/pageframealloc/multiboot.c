#include "multiboot.h"
#include "book.h"
#include "printer.h"
#include "regions.h"
#include <freestanding.h>
#include <stdint.h>
/*
 * You will need two main references for this task. The first is the multiboot
 * specification. The specification lists the exact structure of the tags and
 * details how they tags are passed into the OS. From the tags you need to
 * extract the list of physical memory regions and the location of the ELF
 * section headers. You will need to store the memory regions in an efficient
 * data structure for use by your memory manager.
 */

uint32_t multiboot_pointer;
uint32_t multiboot_magic;

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

typedef enum : uint32_t {
  BOOT_CLI = 1,         // Command-line string
  BOOTLOADER_NAME = 2,  // Bootloader name
  MODULES = 3,          // Loaded modules
  MEMORY = 4,           // Basic memory info
  DEVICE = 5,           // BIOS boot device
  MEM_MAP = 6,          // Memory map
  VBE_INFO = 7,         // VBE framebuffer info
  FRAMEBUFFER_INFO = 8, // Framebuffer info (preferred over VBE)
  ELF_SYMBOLS = 9,      // ELF section header table
  APM_TABLE = 10        // APM BIOS table
} mb_tag_t;

typedef struct {
  uint32_t type;
  uint32_t size;
  // Followed by type-specific data
} mb_tag_header_t;

typedef struct {
  uint64_t base_addr; // phys mem addr
  uint64_t length;    // bytes
  uint32_t
      type; // (only use 1) 1:ram, 3:usable,with acpi, 4:swap, other:reserved
  uint32_t reserved; // 0
} mem_map_entry_t;

////////////////////////////
/// ELF Parsing
////////////////////////////
typedef struct {
  mb_tag_header_t mb_tag;
  uint32_t num_entries;
  uint32_t entry_size;
  uint32_t shndx;
} elf_tag_t;

typedef struct {
  uint32_t name_string_offset; // Nth string in the .shstrtab string table
  uint32_t type;
  uint64_t flags;
  uint64_t virt_addr;   // virtual address, for sections that are loaded
  uint64_t file_offset; // offset of section in the file image
  uint64_t size;        // bytes
  uint32_t link;        // "section index"
  uint32_t info;
  uint64_t address_align; // must be pow2
  uint64_t entry_size;    // bytes
} elf_section_header_t;

static uint64_t dwarves;
static elf_section_header_t dwarf_array[100];
static int shndx;

static char *ret_name_string_by_index(int i) {
  int offset = dwarf_array[i].name_string_offset;
  printk("offset: %d\n", offset);

  char *string_blob = (char *)&dwarf_array[shndx];
  return &string_blob[offset];
}

static void parse_elf_section_headers(mb_tag_header_t *header) {

  elf_tag_t *tag = (elf_tag_t *)header;
  shndx = tag->shndx;

  if (sizeof(elf_section_header_t) != tag->entry_size) {
    ERR_LOOP();
  }

  elf_section_header_t *entry =
      (elf_section_header_t *)((void *)tag + sizeof(*tag));

  // for "elf section header" in entries....
  for (; dwarves < tag->num_entries;) {
    dwarf_array[dwarves++] = *entry++;
  }
};

////////////////////////////
/// MEM_MAP Parsing (phys mem)
////////////////////////////
static int e;
static mem_map_entry_t entries[32];

static void parse_mem_maps(mb_tag_header_t *header) {
  uint32_t *p = ((uint32_t *)header) + sizeof(*header) / sizeof(uint32_t);
  uint32_t entry_size = *(p++);
  uint32_t entry_version = *(p++);
  uint32_t num_entries =
      (header->size - sizeof(mb_tag_header_t) - (sizeof(uint32_t) * 2)) /
      entry_size;

  for (uint32_t i = 0; i < num_entries; i++) {
    entries[e++] = *((mem_map_entry_t *)p);
    p += 6;
  }
};

static void parse_multiboot() {
  if (MULTIBOOT2_BOOTLOADER_MAGIC != multiboot_magic) {
    ERR_LOOP();
  }

  uint32_t *b = (uint32_t *)multiboot_pointer;

  uint32_t size_bytes = b[0];
  if (b[1] != 0) {
    ERR_LOOP();
  }

  mb_tag_header_t *next_tag = (void *)&b[2];

  while ((void *)next_tag < (((void *)b) + size_bytes)) {

    // Round up / account for padding
    uintptr_t addr = (uintptr_t)((uint8_t *)next_tag + next_tag->size);
    next_tag = (void *)((addr + 7) &
                        ~((uintptr_t)7)); // Round up to next multiple of 8

    mb_tag_header_t *header = (mb_tag_header_t *)next_tag;

    switch (header->type) {
    case BOOTLOADER_NAME:
      printk("bootloader is %s\n", ((uint8_t *)header) + 8);
      break;
    case MEM_MAP: {
      parse_mem_maps(header);
    } break;
    case ELF_SYMBOLS: {
      parse_elf_section_headers(header);
    } break;
    case BOOT_CLI:
    case MODULES:
    case MEMORY:
    case DEVICE:
    case VBE_INFO:
    case FRAMEBUFFER_INFO:
    case APM_TABLE:
    default:
      break;
    }
  }
}

////////////////////////////
/// Coalesce Memory Regions
////////////////////////////
static void generate_memory_map(phys_mem_region_t coalesced[100],
                                int *num_coalesced) {
  phys_mem_region_t available[64];
  phys_mem_region_t used[64];
  int num_available = 0;
  int num_used = 0;

  initPageAllocator();

  // generate include list
  uint64_t bytes_available;
  for (int i = 0; i < e; i++) {
    mem_map_entry_t m = entries[i];
    if (m.type == 1) {
      m.base_addr;
      bytes_available += m.length;
      available[num_available++] = (phys_mem_region_t){m.base_addr, m.length};
    }
  }
  printk(" %d bytes available\n", (bytes_available));
  printk(" %d mebibytes available\n", (bytes_available / (1024 * 1024)));

  // check offset is what we expect
  uint64_t offset = dwarf_array[1].virt_addr - dwarf_array[1].file_offset;
  if (offset != VIRT_MEM_OFFSET) {
    // NOTE: this offset should be constant and determined by the page mapping
    // at boot
    ERR_LOOP();
  }

  // Generate exclude list
  uint64_t bytes_used;
  for (int i = 0; i < dwarves; i++) {
    elf_section_header_t d = dwarf_array[i];
    if (d.virt_addr && (d.flags & 0x2)) {
      uint64_t phys_add = d.virt_addr - offset;
      bytes_used += d.size;
      used[num_used++] = (phys_mem_region_t){phys_add, d.size};
    }
  }

  // add Multiboot 2 Data structure to the used list
  uint32_t *b = (uint32_t *)multiboot_pointer;
  uint32_t size_bytes = b[0];
  used[num_used++] =
      (phys_mem_region_t){multiboot_pointer - offset, size_bytes};

  // print fun facts
  printk(" %d bytes used\n", (bytes_used));
  printk(" %d mebibytes used\n", (bytes_used / (1024 * 1024)));

  // coalesce free mem regions
  if (validate_and_coalesce(available, num_available, used, num_used, coalesced,
                            num_coalesced, 100) == 0) {
    ERR_LOOP();
  }
};

void fiftytwo_card_pickup() {
  // parse the mb header and collect entries
  parse_multiboot();

  // generate clean list of non-kernel available memory
  phys_mem_region_t coalesced[100];
  int num_coalesced;
  generate_memory_map(coalesced, &num_coalesced);

  // turn each coalesced region into pages and add to free list (stored in
  // memory , on pages)
  int pages_allocated;
  for (int i = 0; i < num_coalesced; i++) {
    pages_allocated += makePage(coalesced[i]);

    if (pages_allocated > 300)
      break;
  }

  printk("generated %d free pages (%d mebibytes)\n", pages_allocated,
         (pages_allocated * PAGE_SIZE / (1024 * 1024)));

  // the requested demo
  testPageAllocator();
}
