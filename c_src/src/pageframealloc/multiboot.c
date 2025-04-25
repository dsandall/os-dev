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

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

void *read_tag(uint32_t *p);

void parse_multiboot() {
  if (MULTIBOOT2_BOOTLOADER_MAGIC != multiboot_magic) {
    ERR_LOOP();
  }

  uint32_t *b = (uint32_t *)multiboot_pointer;

  uint32_t size_bytes = b[0];
  if (b[1] != 0) {
    ERR_LOOP();
  }

  uintptr_t addr = (uintptr_t)&b[2];
  addr = (addr + 7) & ~((uintptr_t)7); // Round up to next multiple of 8
  void *next_tag = (void *)addr;

  while (next_tag < (((void *)b) + size_bytes)) {
    addr = (uintptr_t)read_tag(next_tag);
    addr = (addr + 7) & ~((uintptr_t)7); // Round up to next multiple of 8
    next_tag = (void *)addr;
  }
}

typedef struct {
  uint64_t base_addr;
  uint64_t length;
  uint32_t type;
  uint32_t reserved;
} mem_map_entry_t;

mem_map_entry_t entries[32];

void *read_tag(uint32_t *p) {
  static int e;

  mb_tag_header_t *header = (mb_tag_header_t *)p;
  void *next_tag_header = (void *)((uint8_t *)header + header->size);

  switch (header->type) {
  case BOOT_CLI:
  case BOOTLOADER_NAME:
  case MODULES:
  case MEMORY:
  case DEVICE:
    p += (header->size / sizeof(uint32_t));
    break;
  case MEM_MAP: {
    p += sizeof(*header) / sizeof(uint32_t);
    uint32_t entry_size = *(p++);
    uint32_t entry_version = *(p++);
    uint32_t num_entries =
        (header->size - sizeof(mb_tag_header_t) - (sizeof(uint32_t) * 2)) /
        entry_size;

    for (uint32_t i = 0; i < num_entries; i++) {
      entries[e++] = *((mem_map_entry_t *)p);
      p += 6;
    }

    break;
  } break;
  case ELF_SYMBOLS: // TODO:
  case VBE_INFO:
  case FRAMEBUFFER_INFO:
  case APM_TABLE:
  default:
    // Unknown tag — safely ignored or logged
    p += (header->size / sizeof(uint32_t));
    break;
  }

  if (next_tag_header != (void *)p) {
    // ERR_LOOP();
  }

  return next_tag_header;
}
