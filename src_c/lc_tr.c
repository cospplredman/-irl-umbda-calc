#include "lc_tr.h"
#include "string.h"

struct lc_tr *lc_tr_ref(char *start, size_t length) {
	struct lc_tr *ret = lc_memory_pool_alloc(memory_pool);
	*ret = (struct lc_tr){.type = LC_REF,
			      .cell.ref.start = start,
			      .cell.ref.length = length,
			      .ref_count = 0};
	return ret;
}

struct lc_tr *lc_tr_app(struct lc_tr *fn, struct lc_tr *arg) {
	struct lc_tr *ret = lc_memory_pool_alloc(memory_pool);
	*ret = (struct lc_tr){.type = LC_APP,
			      .cell.app.fn = fn,
			      .cell.app.arg = arg,
			      .ref_count = 0};
	return ret;
}

struct lc_tr *lc_tr_abs(struct lc_tr *arg, struct lc_tr *body) {
	struct lc_tr *ret = lc_memory_pool_alloc(memory_pool);
	*ret = (struct lc_tr){.type = LC_ABS,
			      .cell.abs.arg = arg,
			      .cell.abs.body = body,
			      .ref_count = 0};
	return ret;
}

struct lc_tr *lc_tr_thunk(struct lc_tr *val, struct lc_tr *eval) {
	struct lc_tr *ret = lc_memory_pool_alloc(memory_pool);
	*ret = (struct lc_tr){.type = LC_THUNK,
			      .cell.thunk.val = val,
			      .cell.thunk.eval = eval,
			      .ref_count = 0};
	return ret;
}

struct lc_tr *lc_tr_c_value(void *val) {
	struct lc_tr *ret = lc_memory_pool_alloc(memory_pool);
	*ret = (struct lc_tr){
	    .type = LC_C_VALUE, .cell.c_value.value = val, .ref_count = 0};
	return ret;
}

struct lc_tr *lc_tr_c_func(struct lc_tr *(*func)(struct lc_tr *)) {
	struct lc_tr *ret = lc_memory_pool_alloc(memory_pool);
	*ret = (struct lc_tr){
	    .type = LC_C_FUNC, .cell.c_func.func = func, .ref_count = 0};
	return ret;
}

void lc_tr_free(struct lc_tr *tr) {
	if (tr == NULL)
		return;

	if (tr->ref_count != 0) {
		tr->ref_count--;
		return;
	}

	switch (tr->type) {
	case LC_REF:
		lc_memory_pool_free(memory_pool, tr);
		break;
	case LC_APP:
		lc_tr_free(tr->cell.app.fn);
		lc_tr_free(tr->cell.app.arg);
		lc_memory_pool_free(memory_pool, tr);
		break;
	case LC_ABS:
		lc_tr_free(tr->cell.abs.arg);
		lc_tr_free(tr->cell.abs.body);
		lc_memory_pool_free(memory_pool, tr);
		break;
	case LC_THUNK:
		lc_tr_free(tr->cell.thunk.val);
		lc_tr_free(tr->cell.thunk.eval);
		lc_memory_pool_free(memory_pool, tr);
		break;
	case LC_C_VALUE:
		free(tr->cell.c_value.value);
		lc_memory_pool_free(memory_pool, tr);
		break;
	case LC_C_FUNC:
		lc_memory_pool_free(memory_pool, tr);
		break;
	}
}

static void print_indent(FILE *out, size_t indent) {
	for (size_t i = 0; i < indent; i++)
		fprintf(out, "  ");
}

void lc_tr_print(FILE *out, struct lc_tr *tr, size_t indent) {
	if (tr == NULL) {
		print_indent(out, indent);
		fprintf(out, "NULL\n");
		return;
	}

	switch (tr->type) {
	case LC_REF:
		print_indent(out, indent);
		fprintf(out, "| REF: %.*s %p %zu\n", (int)tr->cell.ref.length,
			tr->cell.ref.start, (void *)tr, (size_t)tr->ref_count);
		break;
	case LC_APP:
		print_indent(out, indent);
		fprintf(out, "| APP: %p %zu\n", (void *)tr,
			(size_t)tr->ref_count);
		lc_tr_print(out, tr->cell.app.fn, indent + 1);
		lc_tr_print(out, tr->cell.app.arg, indent + 1);
		break;
	case LC_ABS:
		print_indent(out, indent);
		fprintf(out, "| ABS: %p %zu\n", (void *)tr,
			(size_t)tr->ref_count);
		lc_tr_print(out, tr->cell.abs.arg, indent + 1);
		lc_tr_print(out, tr->cell.abs.body, indent + 1);
		break;
	case LC_THUNK:
		print_indent(out, indent);
		fprintf(out, "| THUNK: %p %zu\n", (void *)tr,
			(size_t)tr->ref_count);
		if (tr->cell.thunk.eval != NULL) {
			lc_tr_print(out, tr->cell.thunk.eval, indent + 1);
		} else {
			lc_tr_print(out, tr->cell.thunk.val, indent + 1);
		}
		break;
	case LC_C_VALUE:
		print_indent(out, indent);
		fprintf(out, "| C_VALUE: %p %zu\n", (void *)tr,
			(size_t)tr->ref_count);
		break;
	case LC_C_FUNC:
		print_indent(out, indent);
		fprintf(out, "| C_FUNC: %p %zu\n", (void *)tr,
			(size_t)tr->ref_count);
		break;
	default:
		print_indent(out, indent);
		fprintf(out, "| ?? unkown tree type ??\r\n");
	}
}

int lc_tr_ref_eq(struct lc_tr *a, struct lc_tr *b) {
	if (a->type == LC_REF && b->type == LC_REF) {
		if (a->cell.ref.length == b->cell.ref.length) {
			return a == b ||
			       strncmp(a->cell.ref.start, b->cell.ref.start,
				       a->cell.ref.length) == 0;
		}
	}
	return 0;
}
