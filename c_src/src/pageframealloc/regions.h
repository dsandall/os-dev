#include "book.h"

void validate_and_coalesce(const phys_mem_region_t *available,
                           int available_count, const phys_mem_region_t *used,
                           int used_count, phys_mem_region_t *out,
                           int *out_count);
