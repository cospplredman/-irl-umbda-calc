#include "lc_eval.h"
#include "stdio.h"

static struct lc_tr *lc_tr_beta(struct lc_tr *body, struct lc_tr *ref,
                                struct lc_tr *val) {
  switch (body->type) {
  case LC_REF: {
    if (lc_tr_ref_eq(ref, body)) {
      val->ref_count++;
      return val;
    }

    body->ref_count++;
    return body;
  }
  case LC_APP: {
    return lc_tr_app(lc_tr_beta(body->app.fn, ref, val),
                     lc_tr_beta(body->app.arg, ref, val));
  }
  case LC_ABS: {
    if (lc_tr_ref_eq(ref, body->abs.arg)) {
      body->ref_count++;
      return body;
    }

    body->abs.arg->ref_count++;
    return lc_tr_abs(body->abs.arg, lc_tr_beta(body->abs.body, ref, val));
  }
  case LC_THUNK: {
    body->ref_count++;
    return body;
  }
  case LC_C_VALUE: {
    body->ref_count++;
    return body;
  }
  case LC_C_FUNC: {
    body->ref_count++;
    return body;
  }
  }

  fprintf(stderr, "\r\nreached end of lc_tr_beta\r\n");
  return NULL;
}

static struct lc_tr *lc_tr_call(struct lc_tr *fn, struct lc_tr *arg,
                                struct lc_env *env) {

  switch (fn->type) {
  case LC_ABS: {
    arg->ref_count++;
    struct lc_tr *ret, *thunk = lc_tr_thunk(arg, NULL),
                       *beta = lc_tr_beta(fn->abs.body, fn->abs.arg, thunk);

    ret = eval_lc(beta, env);

    lc_tr_free(thunk);
    lc_tr_free(beta);
    lc_tr_free(fn);

    return ret;
  }
  case LC_C_FUNC: {
    struct lc_tr *val = eval_lc(arg, env), *ret = fn->c_func.func(val);

    lc_tr_free(fn);
    lc_tr_free(val);

    return ret;
  }
  default:
    return lc_tr_app(fn, arg); // STUCK
  }

  // bad case
  fprintf(stderr, "\r\nreached end of lc_tr_call\r\n");
  return NULL;
}

struct lc_tr *eval_lc(struct lc_tr *tr, struct lc_env *env) {
  switch (tr->type) {
  case LC_REF:
    return lc_env_lookup(env, tr);
  case LC_APP:
    return lc_tr_call(eval_lc(tr->app.fn, env), tr->app.arg, env);
  case LC_ABS:
    tr->ref_count++;
    return tr;
  case LC_THUNK: {
    if (tr->thunk.eval == NULL) {
      tr->thunk.eval = eval_lc(tr->thunk.val, env);
    }

    tr->thunk.eval->ref_count++;
    return tr->thunk.eval;
  }
  case LC_C_VALUE: {
    tr->ref_count++;
    return tr;
  }
  case LC_C_FUNC: {
    tr->ref_count++;
    return tr;
  }
  }

  fprintf(stderr, "\r\nreached end of eval_lc %p %p\r\n", tr, env);
  return NULL;
}
