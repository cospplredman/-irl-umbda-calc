#include "lc_tr.h"
#include "string.h"

struct lc_tr *lc_tr_ref(char *start, size_t length) {
  struct lc_tr *ret = malloc(sizeof(struct lc_tr));
  *ret = (struct lc_tr){
      .type = LC_REF, .ref.start = start, .ref.length = length, .ref_count = 0};
  return ret;
}

struct lc_tr *lc_tr_app(struct lc_tr *fn, struct lc_tr *arg) {
  struct lc_tr *ret = malloc(sizeof(struct lc_tr));
  *ret = (struct lc_tr){
      .type = LC_APP, .app.fn = fn, .app.arg = arg, .ref_count = 0};
  return ret;
}

struct lc_tr *lc_tr_abs(struct lc_tr *arg, struct lc_tr *body) {
  struct lc_tr *ret = malloc(sizeof(struct lc_tr));
  *ret = (struct lc_tr){
      .type = LC_ABS, .abs.arg = arg, .abs.body = body, .ref_count = 0};
  return ret;
}

struct lc_tr *lc_tr_thunk(struct lc_tr *val, struct lc_tr *eval) {
  struct lc_tr *ret = malloc(sizeof(struct lc_tr));
  *ret = (struct lc_tr){
      .type = LC_THUNK, .thunk.val = val, .thunk.eval = eval, .ref_count = 0};
  return ret;
}

struct lc_tr *lc_tr_c_value(void *val) {
  struct lc_tr *ret = malloc(sizeof(struct lc_tr));
  *ret =
      (struct lc_tr){.type = LC_C_VALUE, .c_value.value = val, .ref_count = 0};
  return ret;
}

struct lc_tr *lc_tr_c_func(struct lc_tr *(*func)(struct lc_tr *)) {
  struct lc_tr *ret = malloc(sizeof(struct lc_tr));
  *ret = (struct lc_tr){.type = LC_C_FUNC, .c_func.func = func, .ref_count = 0};
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
    free(tr);
    break;
  case LC_APP:
    lc_tr_free(tr->app.fn);
    lc_tr_free(tr->app.arg);
    free(tr);
    break;
  case LC_ABS:
    lc_tr_free(tr->abs.arg);
    lc_tr_free(tr->abs.body);
    free(tr);
    break;
  case LC_THUNK:
    lc_tr_free(tr->thunk.val);
    lc_tr_free(tr->thunk.eval);
    free(tr);
    break;
  case LC_C_VALUE:
    free(tr->c_value.value);
    free(tr);
    break;
  case LC_C_FUNC:
    free(tr);
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
    fprintf(out, "| REF: %.*s %p %zu\n", (int)tr->ref.length, tr->ref.start, tr,
            tr->ref_count);
    break;
  case LC_APP:
    print_indent(out, indent);
    fprintf(out, "| APP: %p %zu\n", tr, tr->ref_count);
    lc_tr_print(out, tr->app.fn, indent + 1);
    lc_tr_print(out, tr->app.arg, indent + 1);
    break;
  case LC_ABS:
    print_indent(out, indent);
    fprintf(out, "| ABS: %p %zu\n", tr, tr->ref_count);
    lc_tr_print(out, tr->abs.arg, indent + 1);
    lc_tr_print(out, tr->abs.body, indent + 1);
    break;
  case LC_THUNK:
    print_indent(out, indent);
    fprintf(out, "| THUNK: %p %zu\n", tr, tr->ref_count);
    if (tr->thunk.eval != NULL) {
      lc_tr_print(out, tr->thunk.eval, indent + 1);
    } else {
      lc_tr_print(out, tr->thunk.val, indent + 1);
    }
    break;
  case LC_C_VALUE:
    print_indent(out, indent);
    fprintf(out, "| C_VALUE: %p %zu\n", tr, tr->ref_count);
    break;
  case LC_C_FUNC:
    print_indent(out, indent);
    fprintf(out, "| C_FUNC: %p %zu\n", tr, tr->ref_count);
    break;
  default:
    print_indent(out, indent);
    fprintf(out, "| ?? unkown tree type ??\r\n");
  }
}

int lc_tr_ref_eq(struct lc_tr *a, struct lc_tr *b) {
  if (a->type == LC_REF && b->type == LC_REF) {
    if (a->ref.length == b->ref.length) {
      return strncmp(a->ref.start, b->ref.start, a->ref.length) == 0;
    }
  }
  return 0;
}
