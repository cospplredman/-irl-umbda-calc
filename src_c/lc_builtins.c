#include "lc_builtins.h"
#include "lc_tr.h"

struct lc_tr *lc_c_print_tr(struct lc_tr *arg) {
  lc_tr_print(stdout, arg, 0);
  arg->ref_count++;
  return arg;
}

struct lc_tr *lc_c_print_cstr(struct lc_tr *arg) {
  if (arg->type == LC_C_VALUE) {
    printf("%s", (char *)arg->cell.c_value.value);
    arg->ref_count++;
    return arg;
  }

  fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
  arg->ref_count++;
  return arg;
}

struct lc_tr *lc_c_print_i(struct lc_tr *arg) {
  if (arg->type == LC_C_VALUE) {
    printf("\n\n%d\n\n", *(int *)arg->cell.c_value.value);
    arg->ref_count++;
    return arg;
  }

  fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
  arg->ref_count++;
  return arg;
}

struct lc_tr *lc_c_atoi(struct lc_tr *arg) {
  if (arg->type == LC_C_VALUE) {
    int *box = malloc(sizeof(int));
    *box = atoi((char *)arg->cell.c_value.value);
    return lc_tr_c_value(box);
  }

  fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
  arg->ref_count++;
  return arg;
}

struct lc_tr *lc_c_2n(struct lc_tr *arg) {
  if (arg->type == LC_C_VALUE) {
    int *box = malloc(sizeof(int));
    *box = *((int *)arg->cell.c_value.value) * 2;
    return lc_tr_c_value(box);
  }

  fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
  arg->ref_count++;
  return arg;
}

struct lc_tr *lc_c_inc(struct lc_tr *arg) {
  if (arg->type == LC_C_VALUE) {
    int *box = malloc(sizeof(int));
    *box = *((int *)arg->cell.c_value.value) + 1;
    return lc_tr_c_value(box);
  }

  fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
  arg->ref_count++;
  return arg;
}
