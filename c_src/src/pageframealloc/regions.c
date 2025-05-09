#include "book.h"
#include "freestanding.h"
#include <stdint.h>

static bool region_contains(const phys_mem_region_t *a,
                            const phys_mem_region_t *b) {
  return b->base >= a->base && (b->base + b->size) <= (a->base + a->size);
}

// Swap two regions
static void swap(phys_mem_region_t *a, phys_mem_region_t *b) {
  phys_mem_region_t tmp = *a;
  *a = *b;
  *b = tmp;
}

// Simple in-place bubble sort by base address (for small N)
static void sort_regions(phys_mem_region_t *regions, int count) {
  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - 1 - i; j++) {
      if (regions[j].base > regions[j + 1].base) {
        swap(&regions[j], &regions[j + 1]);
      }
    }
  }
}

// Subtract `used` from `avail` and store the resulting (up to 2) regions in
// `out`. Returns the number of resulting regions (0, 1, or 2).
static int subtract_region(const phys_mem_region_t *avail,
                           const phys_mem_region_t *used,
                           phys_mem_region_t *out) {
  uint64_t a_start = avail->base;
  uint64_t a_end = avail->base + avail->size;
  uint64_t u_start = used->base;
  uint64_t u_end = used->base + used->size;

  if (u_end <= a_start || u_start >= a_end)
    return 1, out[0] = *avail, 1; // no overlap

  int count = 0;
  if (u_start > a_start) {
    out[count].base = a_start;
    out[count].size = (uint32_t)(u_start - a_start);
    count++;
  }
  if (u_end < a_end) {
    out[count].base = u_end;
    out[count].size = (uint32_t)(a_end - u_end);
    count++;
  }
  return count;
}

#define MAX_REGIONS 256
static phys_mem_region_t tmp[MAX_REGIONS * 2];
static phys_mem_region_t new_tmp[MAX_REGIONS * 2];
int validate_and_coalesce(const phys_mem_region_t *available,
                          int available_count, const phys_mem_region_t *used,
                          int used_count, phys_mem_region_t *out,
                          int *out_count, int out_max) {
  // 1. Validate
  for (int i = 0; i < used_count; i++) {
    bool found = false;
    for (int j = 0; j < available_count; j++) {
      if (region_contains(&available[j], &used[i])) {
        found = true;
        break;
      }
    }
    if (!found)
      return 0;
  }

  // 2. Copy available into temp array
  int tmp_count = 0;
  for (int i = 0; i < available_count; i++)
    tmp[tmp_count++] = available[i];

  // 3. Subtract all used regions from tmp
  for (int i = 0; i < used_count; i++) {
    int new_count = 0;

    for (int j = 0; j < tmp_count; j++) {
      phys_mem_region_t parts[2];
      int n = subtract_region(&tmp[j], &used[i], parts);
      for (int k = 0; k < n; k++) {
        new_tmp[new_count++] = parts[k];
      }
    }

    // Update tmp list
    for (int j = 0; j < new_count; j++)
      tmp[j] = new_tmp[j];
    tmp_count = new_count;
  }

  // 4. Sort
  sort_regions(tmp, tmp_count);

  // 5. Coalesce
  int out_idx = 0;
  for (int i = 0; i < tmp_count; i++) {
    if (tmp[i].size == 0)
      continue;

    if (out_idx == 0) {
      out[out_idx++] = tmp[i];
    } else {
      phys_mem_region_t *prev = &out[out_idx - 1];
      if (prev->base + prev->size == tmp[i].base) {
        prev->size += tmp[i].size;
      } else {
        if (out_idx >= out_max)
          return 0;
        out[out_idx++] = tmp[i];
      }
    }
  }

  // 6. Filter out regions smaller than 4096 bytes
  int final_count = 0;
  for (int i = 0; i < out_idx; i++) {
    if (out[i].size >= 4096) {
      out[final_count++] = out[i];
    }
  }
  *out_count = final_count;

  // Optional: reporting
  uint64_t bytes_free = 0;
  for (int i = 0; i < final_count; i++)
    bytes_free += out[i].size;

  printk("%llu bytes in free table\n", bytes_free);
  printk("%llu mebibytes in free table\n", bytes_free / (1024 * 1024));

  return 1;
}
