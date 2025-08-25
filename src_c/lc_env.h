#ifndef GC_LC_LC_ENV_H
#define GC_LC_LC_ENV_H

// TODO turn this into a search tree / trie / hash table of some kind

struct lc_env {
	struct lc_tr *ref;
	struct lc_tr *val;
	struct lc_env *next;
};

struct lc_env *lc_env(struct lc_tr *ref, struct lc_tr *val,
		      struct lc_env *next);
void lc_env_free(struct lc_env *env);
struct lc_tr *lc_env_lookup(struct lc_env *env, struct lc_tr *ref);

struct lc_trie_env {
	struct lc_tr *val;
	struct lc_trie_env *next[256];
};

struct lc_trie_env *lc_trie_env();
void lc_trie_env_free(struct lc_trie_env *env);

void lc_trie_env_set(struct lc_trie_env *env, struct lc_tr *ref,
		     struct lc_tr *val);
struct lc_tr *lc_trie_env_get(struct lc_trie_env *env, struct lc_tr *ref);
struct lc_tr **lc_trie_env_get_ref(struct lc_trie_env *env, struct lc_tr *ref);

#endif
