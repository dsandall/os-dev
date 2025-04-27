#include "multiboot.h"
#include "book.h"
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

typedef struct {
  uint16_t num_entries;
  uint16_t entry_size;
  uint16_t shndx;
  uint16_t reserved;
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

static int dwarves;
elf_section_header_t dwarf_array[100];

static void parse_elf_section_headers(mb_tag_header_t *header) {
  uint32_t *p = ((uint32_t *)header) + sizeof(*header) / sizeof(uint32_t);
  elf_tag_t tag = *(elf_tag_t *)p;

  // TODO: for "elf section header" in entries....
  p += sizeof(tag) / sizeof(uint32_t);
  elf_section_header_t *entry = (elf_section_header_t *)p;
  for (; dwarves < tag.num_entries;) {
    dwarf_array[dwarves++] = *entry++;
  }

  dwarf_array[tag.shndx];

  for (int i = 0; i < dwarves; i++) {
    printk("type: %d\n", dwarf_array[i].type);
  }
};

static int e;
mem_map_entry_t entries[32];

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

static void read_tag(mb_tag_header_t *header) {
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

static void generate_memory_map() {

  initPageAllocator();

  printk("%lu\n", &entries[0]);

  // generate include list
  uint64_t available;
  for (int i = 0; i < e; i++) {
    mem_map_entry_t m = entries[i];
    if (m.type == 1) {
      m.base_addr;
      available += m.length;
      const phys_mem_region_t add = {m.base_addr, m.length};
      makePage(add);
    }
  }
  printk(" %d kibibytes available\n", available / 1024);

  // Generate exclude list
  uint64_t offset = dwarf_array[1].virt_addr - dwarf_array[1].file_offset;
  uint64_t used;
  for (int i = 0; i < dwarves; i++) {
    elf_section_header_t d = dwarf_array[i];
    if (d.virt_addr && (d.flags & 0x2)) {
      uint64_t phys_add = d.virt_addr - offset;
      used += d.size;
      const phys_mem_region_t remove = {phys_add, d.size};
      takePage(remove);
    }
  }
  printk(" %d kibibytes used\n", used / 1024);

  ERR_LOOP(); // TODO:
};

void parse_multiboot() {
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

    read_tag(next_tag);
  }

  generate_memory_map();
}
