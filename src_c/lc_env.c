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

struct lc_trie_env *lc_trie_env() {
	struct lc_trie_env *ret = malloc(sizeof(struct lc_trie_env));

	ret->val = NULL;

	for (size_t i = 0; i < 256; i++) {
		ret->next[i] = NULL;
	}

	return ret;
}

void lc_trie_env_free(struct lc_trie_env *env) {
	if (env == NULL)
		return;

	lc_tr_free(env->val);

	for (size_t i = 0; i < 256; i++) {
		lc_trie_env_free(env->next[i]);
	}

	free(env);
}

struct lc_tr **lc_trie_env_get_ref(struct lc_trie_env *env, struct lc_tr *ref) {
	size_t str_len = ref->cell.ref.length;
	char *str = ref->cell.ref.start;

	for (size_t i = 0; i < str_len; i++) {
		if (env->next[(size_t)str[i]] == NULL) {
			env->next[(size_t)str[i]] = lc_trie_env();
		}

		env = env->next[(size_t)str[i]];
	}

	return &env->val;
}

void lc_trie_env_set(struct lc_trie_env *env, struct lc_tr *ref,
		     struct lc_tr *val) {
	*lc_trie_env_get_ref(env, ref) = val;
}

struct lc_tr *lc_trie_env_get(struct lc_trie_env *env, struct lc_tr *ref) {
	size_t str_len = ref->cell.ref.length;
	char *str = ref->cell.ref.start;

	for (size_t i = 0; i < str_len; i++) {
		if (env->next[(size_t)str[i]] == NULL)
			return NULL;

		env = env->next[(size_t)str[i]];
	}

	return env->val;
}
