#include "book.h"
#include "freestanding.h"
#include "paging.h"
#include "printer.h"
#include "rejigger_paging.h"
#include "virtpage_alloc.h"
#include <stddef.h>
#include <stdint.h>

// there is 1 pool for each size chunk
struct KmallocPool {
  size_t max_size;
  uint64_t avail;
  struct FreeList *head;
};

// list of pointers (LIFO, like a stack)
struct FreeList {
  struct FreeList *next;
};

// header placed before the address returned by kmalloc
// necessary info for returning the freed block to the pool
struct KmallocExtra {
  struct KmallocPool *pool;
  size_t size;
};

#define NEED_SIZE(size) (size + sizeof(struct KmallocExtra))
#define CIELING_DIV(num, den) ((num / den) + (num % den ? 1 : 0))
#define CUSTOM_POOL_MAGIC (struct KmallocPool *)0x66667777
#define NUM_STATIC_POOLS 7 + 1

struct KmallocPool staticPools[NUM_STATIC_POOLS] = {
    {.max_size = 0, .head = 0},    {.max_size = 32, .head = 0},
    {.max_size = 64, .head = 0},   {.max_size = 128, .head = 0},
    {.max_size = 256, .head = 0},  {.max_size = 512, .head = 0},
    {.max_size = 1024, .head = 0}, {.max_size = 2048, .head = 0}};

static uint8_t find_suitable_pool(size_t need_size) {
  for (int i = 1; i < NUM_STATIC_POOLS; i++) {
    if (need_size <= staticPools[i].max_size)
      return i;
  }
  return 0;
}

static struct KmallocExtra *pop(struct KmallocPool *pool) {
  virt_addr_t ret = {.raw = (uint64_t)pool->head};
  pool->head = pool->head->next;
  pool->avail--;
  return (struct KmallocExtra *)ret.raw;
};

static void push(struct KmallocExtra *v, struct KmallocPool *pool) {
  // ASSERT pool exists, this should only be called on static pools
  struct FreeList *temp = pool->head;
  pool->head = (void *)v;
  pool->head->next = temp;
  pool->avail++;
};

void kfree(virt_addr_t addr) {
  // first, access the header for associated pointer
  // if it came from a pool, add the block to the pool stack
  // if it was special, free pages based on the associated size

  ASSERT(is_in_kheap(addr));
  ASSERT(has_been_demanded(addr)); // and it has been handed out

  return; // WARN: not returning memory!

  struct KmallocExtra *header =
      (struct KmallocExtra *)(addr.raw) - (uint64_t)sizeof(struct KmallocExtra);

  if (header->pool != CUSTOM_POOL_MAGIC) {
    // memory came from a pool, add the block to the pool stack
    push(header, header->pool);
  } else {
    // special allocation, deallocate pages
    uint64_t pages = CIELING_DIV(header->size, PAGE_SIZE);
    virt_addr_t start = {.raw = (uint64_t)header};

    for (; pages > 0; pages--) {
      MMU_free_page(start);
      start.raw -= (uint64_t)PAGE_SIZE;
    }
  }
};

virt_addr_t kmalloc(size_t size) {
  // request number of bytes (contiguous in virt mem)
  // return virt pointer
  //
  // find pool with smallest suitable size (accounting for size of header)
  //
  // if no suitable pool exists(say, a large allocation), make some special
  // pages
  // if pool exists, but has no blocks, alloc more page(s) for pool
  //    and chunk page then add to list
  //
  // prior to return, place the header, and return a pointer (adjusted beyond
  // the header)

  ASSERT(size);

  const size_t ns = NEED_SIZE(size);
  const uint8_t pool_index = find_suitable_pool(ns);

  // return
  struct KmallocPool *p;
  struct KmallocExtra *ret = 0;
  size_t allocated_size;

  // tracek("attempting to allocate %lu bytes\n", ns);

  if (pool_index) {
    // fits into existing pools
    p = &staticPools[pool_index];

    // allocate more pages if needed
    if (!p->avail) {
      virt_addr_t new_pool_page = MMU_alloc_page();

      uint64_t count = PAGE_SIZE;
      while (count > 0) {
        push((struct KmallocExtra *)new_pool_page.raw, p);
        count -= p->max_size;
        new_pool_page.raw += p->max_size;
      }
    }

    // get available vaddr from pool
    ret = pop(p);
    allocated_size = p->max_size;

    // tracek("allocated from static pool of size %lu\n", allocated_size);
  } else {
    // custom pool
    p = CUSTOM_POOL_MAGIC;

    // allocate virt pages
    uint64_t necessary_pages = CIELING_DIV(ns, PAGE_SIZE);

    for (uint64_t i = 0; i < necessary_pages; i++) {
      virt_addr_t v = MMU_alloc_page();

      if (!ret)
        ret = (struct KmallocExtra *)v.raw; // take first page addr as ret
    }

    allocated_size = necessary_pages * PAGE_SIZE;

    // tracek("allocated custom pool of size %lu\n", allocated_size);
  }

  // place the header
  struct KmallocExtra header = {.pool = p, .size = allocated_size};
  *ret = header;

  // return incremented address
  ASSERT(is_in_kheap((virt_addr_t){.raw = (uint64_t)ret}));

  virt_addr_t vret =
      (virt_addr_t){.raw = (uint64_t)ret + sizeof(struct KmallocExtra)};

  tracek("handed out: %p from pool %d with requested size %lx\n", vret,
         pool_index, size);
  return vret;
};
