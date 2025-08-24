#include "lc_builtins.h"
#include "lc_env.h"
#include "lc_eval.h"
#include "lc_parse.h"
#include "lc_tr.h"

int main(int argc, char **argv) {
  char *str = "";

  struct lc_env *env =
      // TODO: Replace this with a static lookup table. possible with a linked
      // list at the bottom for user extended functions/whatever term I am about
      // to get um actuallied with.
      lc_env(
          lc_tr_ref("print_tr", 8), lc_tr_c_func(lc_c_print_tr),
          lc_env(
              lc_tr_ref("print_cstr", 10), lc_tr_c_func(lc_c_print_cstr),
              lc_env(
                  lc_tr_ref("atoi", 4), lc_tr_c_func(lc_c_atoi),
                  lc_env(lc_tr_ref("print_i", 7), lc_tr_c_func(lc_c_print_i),
                         lc_env(lc_tr_ref("c_inc", 5), lc_tr_c_func(lc_c_inc),
                                lc_env(lc_tr_ref("c_2n", 4),
                                       lc_tr_c_func(lc_c_2n), NULL))))));

  if (argc == 2) {
    str = read_file(argv[1]);
    if (str == NULL) {
      perror("lc");
      return 1;
    }
  } else {
    fprintf(stderr, "\r\nlc [file]\r\n");
    return 1;
  }

  struct lc_tr *tr = parse(str);
  // lc_tr_print(stderr, tr, 0);

  if (tr == NULL) {
    fprintf(stderr, "\r\n failed to parse %s\r\n", argv[1]);
    return 1;
  }

  struct lc_tr *ret = eval_lc(tr, env);

  lc_tr_free(ret);
  lc_tr_free(tr);
  lc_env_free(env);
  free(str);

  return 0;
}
