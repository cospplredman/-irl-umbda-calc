#include "lc_env.h"
#include "lc_tr.h"

struct lc_env *lc_env(struct lc_tr *ref, struct lc_tr *val,
                      struct lc_env *next) {
  struct lc_env *ret = malloc(sizeof(struct lc_env));
  ret->ref = ref;
  ret->val = val;
  ret->next = next;
  return ret;
}

void lc_env_free(struct lc_env *env) {
  if (env == NULL)
    return;

  lc_tr_free(env->ref);
  lc_tr_free(env->val);
  lc_env_free(env->next);
  free(env);
}

struct lc_tr *lc_env_lookup(struct lc_env *env, struct lc_tr *ref) {
  if (env == NULL) {
    ref->ref_count++;
    return ref;
  }

  if (lc_tr_ref_eq(ref, env->ref)) {
    env->val->ref_count++;
    return env->val;
  }

  return lc_env_lookup(env->next, ref);
}
