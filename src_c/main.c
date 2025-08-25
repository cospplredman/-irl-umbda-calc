#include "lc_builtins.h"
#include "lc_env.h"
#include "lc_eval.h"
#include "lc_memory_pool.h"
#include "lc_parse.h"
#include "lc_tr.h"

// TODO remove all globals >:I
struct lc_memory_pool *memory_pool;
struct lc_trie_env *sym_table;

int main(int argc, char **argv) {
	char *str = "";

	memory_pool = lc_memory_pool(1u << 24u);
	sym_table = lc_trie_env();

	struct lc_trie_env *env = lc_trie_env();
	lc_trie_env_set(env, lc_tr_ref("print_tr", 8),
			lc_tr_c_func(lc_c_print_tr));
	lc_trie_env_set(env, lc_tr_ref("print_cstr", 10),
			lc_tr_c_func(lc_c_print_cstr));
	lc_trie_env_set(env, lc_tr_ref("atoi", 4), lc_tr_c_func(lc_c_atoi));
	lc_trie_env_set(env, lc_tr_ref("print_i", 7),
			lc_tr_c_func(lc_c_print_i));
	lc_trie_env_set(env, lc_tr_ref("c_inc", 5), lc_tr_c_func(lc_c_inc));
	lc_trie_env_set(env, lc_tr_ref("c_2n", 4), lc_tr_c_func(lc_c_2n));

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

	if (tr == NULL) {
		fprintf(stderr, "\r\n failed to parse %s\r\n", argv[1]);
		return 1;
	}

	struct lc_tr *tr_with_env = lc_tr_beta_env(tr, env);

	//>:3
	struct lc_tr *ret = eval_lc(tr_with_env);

	lc_tr_free(ret);
	lc_tr_free(tr);
	lc_tr_free(tr_with_env);
	lc_trie_env_free(env);
	lc_trie_env_free(sym_table);
	lc_memory_pool_free_pool(memory_pool);
	free(str);

	return 0;
}
