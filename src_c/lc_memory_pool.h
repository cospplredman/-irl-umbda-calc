#ifndef GC_LC_LC_MEMORY_POOL_H
#define GC_LC_LC_MEMORY_POOL_H

#include "lc_tr.h"
#include "stddef.h"

struct lc_memory_pool {
  size_t size;
  struct lc_tr *buffer;

  size_t free_list_end;
  size_t *free_list;
};

extern struct lc_memory_pool *memory_pool;

struct lc_memory_pool *lc_memory_pool(size_t initial_size);
void lc_memory_pool_free_pool(struct lc_memory_pool *pool);
struct lc_tr *lc_memory_pool_alloc(struct lc_memory_pool *pool);
void lc_memory_pool_free(struct lc_memory_pool *pool, struct lc_tr *tree);

#endif
