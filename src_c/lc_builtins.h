#ifndef GC_LC_LC_BUILTINS_H
#define GC_LC_LC_BUILTINS_H

struct lc_tr *lc_c_print_tr(struct lc_tr *arg);
struct lc_tr *lc_c_print_cstr(struct lc_tr *arg);
struct lc_tr *lc_c_print_i(struct lc_tr *arg);
struct lc_tr *lc_c_atoi(struct lc_tr *arg);
struct lc_tr *lc_c_2n(struct lc_tr *arg);
struct lc_tr *lc_c_inc(struct lc_tr *arg);

#endif
