#include "lc_eval.h"
#include "stdio.h"

static struct lc_tr *lc_tr_beta(struct lc_tr *body, struct lc_tr *ref,
				struct lc_tr *val) {
	switch (body->type) {
	case LC_APP: {
		// TODO: makes a copy which may not be what i want...
		return lc_tr_app(lc_tr_beta(body->cell.app.fn, ref, val),
				 lc_tr_beta(body->cell.app.arg, ref, val));
	}
	case LC_ABS: {
		if (lc_tr_ref_eq(ref, body->cell.abs.arg)) {
			body->ref_count++;
			return body;
		}

		body->cell.abs.arg->ref_count++;
		return lc_tr_abs(body->cell.abs.arg,
				 lc_tr_beta(body->cell.abs.body, ref, val));
	}
	case LC_REF: {
		if (lc_tr_ref_eq(ref, body)) {
			val->ref_count++;
			return val;
		}
	} /* fallthrough */
	case LC_THUNK:
	case LC_C_VALUE:
	case LC_C_FUNC: {
		body->ref_count++;
		return body;
	}
	}

	fprintf(stderr, "\r\nreached end of %s\r\n", __func__);
	return NULL;
}

struct lc_tr *lc_tr_beta_env(struct lc_tr *body, struct lc_trie_env *env) {
	switch (body->type) {
	case LC_APP: {
		// TODO: makes a copy which may not be what i want...
		return lc_tr_app(lc_tr_beta_env(body->cell.app.fn, env),
				 lc_tr_beta_env(body->cell.app.arg, env));
	}
	case LC_ABS: {
		if (lc_trie_env_get(env, body->cell.abs.arg) != NULL) {
			body->ref_count++;
			return body;
		}

		body->cell.abs.arg->ref_count++;
		return lc_tr_abs(body->cell.abs.arg,
				 lc_tr_beta_env(body->cell.abs.body, env));
	}
	case LC_REF: {
		struct lc_tr *val = lc_trie_env_get(env, body);
		if (val != NULL) {
			val->ref_count++;
			return val;
		}
	} /* fallthrough */
	case LC_THUNK:
	case LC_C_VALUE:
	case LC_C_FUNC: {
		body->ref_count++;
		return body;
	}
	}

	fprintf(stderr, "\r\nreached end of %s\r\n", __func__);
	return NULL;
}

static struct lc_tr *lc_tr_call(struct lc_tr *fn, struct lc_tr *arg) {
	switch (fn->type) {
	case LC_ABS: {
		arg->ref_count++;
		struct lc_tr *ret, *thunk = lc_tr_thunk(arg, NULL),
				   *beta = lc_tr_beta(fn->cell.abs.body,
						      fn->cell.abs.arg, thunk);

		ret = eval_lc(beta);

		lc_tr_free(thunk);
		lc_tr_free(beta);
		lc_tr_free(fn);

		return ret;
	}
	case LC_C_FUNC: {
		struct lc_tr *val = eval_lc(arg),
			     *ret = fn->cell.c_func.func(val);

		lc_tr_free(fn);
		lc_tr_free(val);

		return ret;
	}
	default:
		fn->ref_count++;
		arg->ref_count++;
		return lc_tr_app(fn, arg); // STUCK
	}

	// bad case
	fprintf(stderr, "\r\nreached end of %s\r\n", __func__);
	return NULL;
}

struct lc_tr *eval_lc(struct lc_tr *tr) {
	switch (tr->type) {
	case LC_APP:
		return lc_tr_call(eval_lc(tr->cell.app.fn), tr->cell.app.arg);
	case LC_THUNK: {
		if (tr->cell.thunk.eval == NULL) {
			tr->cell.thunk.eval = eval_lc(tr->cell.thunk.val);
			lc_tr_free(tr->cell.thunk.val);
			tr->cell.thunk.val = NULL;
		}

		tr->cell.thunk.eval->ref_count++;
		return tr->cell.thunk.eval;
	}
	case LC_REF:
	case LC_ABS:
	case LC_C_VALUE:
	case LC_C_FUNC: {
		tr->ref_count++;
		return tr;
	}
	}

	fprintf(stderr, "\r\nreached end of %s\r\n", __func__);
	return NULL;
}
