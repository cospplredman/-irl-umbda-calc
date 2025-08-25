#include "lc_memory_pool.h"

struct lc_memory_pool *lc_memory_pool(size_t initial_size) {
	struct lc_memory_pool *ret = malloc(sizeof(struct lc_memory_pool));

	*ret = (struct lc_memory_pool){
	    .size = initial_size,
	    .buffer = malloc(initial_size * sizeof(struct lc_tr)),
	    .free_list_end = initial_size,
	    .free_list = malloc(initial_size * sizeof(size_t))};

	for (size_t i = 0; i < ret->free_list_end; i++) {
		ret->free_list[i] = i;
	}

	return ret;
}

void lc_memory_pool_free_pool(struct lc_memory_pool *pool) {
	free(pool->buffer);
	free(pool->free_list);
	free(pool);
}

static void free_list_push(struct lc_memory_pool *pool, size_t val) {
	pool->free_list[pool->free_list_end++] = val;
}

static size_t free_list_pop(struct lc_memory_pool *pool) {
	return pool->free_list[--pool->free_list_end];
}

struct lc_tr *lc_memory_pool_alloc(struct lc_memory_pool *pool) {
	if (pool->free_list_end > 0) {
		return pool->buffer + free_list_pop(pool);
	}

	// ran out of pool :/
	return malloc(sizeof(struct lc_tr));
}

void lc_memory_pool_free(struct lc_memory_pool *pool, struct lc_tr *tree) {
	if (tree >= pool->buffer && tree <= pool->buffer + pool->size) {
		free_list_push(pool, tree - pool->buffer);
	} else {
		free(tree);
	}
}
