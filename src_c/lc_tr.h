#ifndef GC_LC_LC_TR_H
#define GC_LC_LC_TR_H
#include "stdio.h"
#include "stdlib.h"

enum lc_tr_type { LC_REF, LC_APP, LC_ABS, LC_THUNK, LC_C_VALUE, LC_C_FUNC };

struct lc_tr {
  enum lc_tr_type type;
  size_t ref_count;

  union {
    struct lc_ref {
      char *start;
      size_t length;
    } ref;
    struct lc_app {
      struct lc_tr *fn, *arg;
    } app;
    struct lc_abs {
      struct lc_tr *arg, *body;
    } abs;
    struct lc_thunk {
      struct lc_tr *val, *eval;
    } thunk;

    struct lc_c_value {
      void *value;
    } c_value;
    struct lc_c_func {
      struct lc_tr *(*func)(struct lc_tr *arg);
    } c_func; // TODO maybe add an arity tag?
  } cell;
};

struct lc_tr *lc_tr_ref(char *start, size_t length);
struct lc_tr *lc_tr_app(struct lc_tr *fn, struct lc_tr *arg);
struct lc_tr *lc_tr_abs(struct lc_tr *arg, struct lc_tr *body);
struct lc_tr *lc_tr_thunk(struct lc_tr *val, struct lc_tr *eval);
struct lc_tr *lc_tr_c_value(void *val);
struct lc_tr *lc_tr_c_func(struct lc_tr *(*func)(struct lc_tr *));
void lc_tr_free(struct lc_tr *tr);
void lc_tr_print(FILE *out, struct lc_tr *tr, size_t indent);
int lc_tr_ref_eq(struct lc_tr *a, struct lc_tr *b);

#endif
