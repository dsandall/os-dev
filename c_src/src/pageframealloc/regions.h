
#ifndef REGIONS_H
#define REGIONS_H

#include "freestanding.h"

typedef struct {
  uint64_t base;
  uint32_t size;
} phys_mem_region_t;

void validate_and_coalesce(const phys_mem_region_t *available,
                           int available_count, const phys_mem_region_t *used,
                           int used_count, phys_mem_region_t *out,
                           int *out_count);

#endif // REGIONS_H
