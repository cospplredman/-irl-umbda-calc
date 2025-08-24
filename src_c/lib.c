/**
 * A monolib of the mono-main.c file provided by cos.
 * Used as a staging ground to rip out and isolate reusable components.
 */

#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "ctype.h"

enum lc_tr_type
{
 LC_REF,
 LC_APP,
 LC_ABS,
 LC_THUNK,
 LC_C_VALUE,
 LC_C_FUNC
};

struct lc_tr
{
 enum lc_tr_type type;
 size_t ref_count;

 union
 {
  struct lc_ref
  {
   char *start;
   size_t length;
  } ref;
  struct lc_app
  {
   struct lc_tr *fn, *arg;
  } app;
  struct lc_abs
  {
   struct lc_tr *arg, *body;
  } abs;
  struct lc_thunk
  {
   struct lc_tr *val, *eval;
  } thunk;

  struct lc_c_value
  {
   void *value;
  } c_value;
  struct lc_c_func
  {
   struct lc_tr *(*func)(struct lc_tr *arg);
  } c_func; // TODO maybe add an arity tag?
 };
};

struct lc_tr *lc_tr_ref(char *start, size_t length)
{
 struct lc_tr *ret = malloc(sizeof(struct lc_tr));
 *ret = (struct lc_tr){.type = LC_REF, .ref.start = start, .ref.length = length, .ref_count = 0};
 return ret;
}

struct lc_tr *lc_tr_app(struct lc_tr *fn, struct lc_tr *arg)
{
 struct lc_tr *ret = malloc(sizeof(struct lc_tr));
 *ret = (struct lc_tr){.type = LC_APP, .app.fn = fn, .app.arg = arg, .ref_count = 0};
 return ret;
}

struct lc_tr *lc_tr_abs(struct lc_tr *arg, struct lc_tr *body)
{
 struct lc_tr *ret = malloc(sizeof(struct lc_tr));
 *ret = (struct lc_tr){.type = LC_ABS, .abs.arg = arg, .abs.body = body, .ref_count = 0};
 return ret;
}

struct lc_tr *lc_tr_thunk(struct lc_tr *val, struct lc_tr *eval)
{
 struct lc_tr *ret = malloc(sizeof(struct lc_tr));
 *ret = (struct lc_tr){.type = LC_THUNK, .thunk.val = val, .thunk.eval = eval, .ref_count = 0};
 return ret;
}

struct lc_tr *lc_tr_c_value(void *val)
{
 struct lc_tr *ret = malloc(sizeof(struct lc_tr));
 *ret = (struct lc_tr){.type = LC_C_VALUE, .c_value.value = val, .ref_count = 0};
 return ret;
}

struct lc_tr *lc_tr_c_func(struct lc_tr *(*func)(struct lc_tr *))
{
 struct lc_tr *ret = malloc(sizeof(struct lc_tr));
 *ret = (struct lc_tr){.type = LC_C_FUNC, .c_func.func = func, .ref_count = 0};
 return ret;
}

void lc_tr_free(struct lc_tr *tr)
{
 if (tr == NULL)
  return;

 if (tr->ref_count != 0)
 {
  tr->ref_count--;
  return;
 }

 switch (tr->type)
 {
 case LC_REF:
  free(tr);
  break;
 case LC_APP:
  lc_tr_free(tr->app.fn);
  lc_tr_free(tr->app.arg);
  free(tr);
  break;
 case LC_ABS:
  lc_tr_free(tr->abs.arg);
  lc_tr_free(tr->abs.body);
  free(tr);
  break;
 case LC_THUNK:
  lc_tr_free(tr->thunk.val);
  lc_tr_free(tr->thunk.eval);
  free(tr);
  break;
 case LC_C_VALUE:
  free(tr->c_value.value);
  free(tr);
  break;
 case LC_C_FUNC:
  free(tr);
  break;
 }
}

static void print_indent(FILE *out, size_t indent)
{
 for (size_t i = 0; i < indent; i++)
  fprintf(out, "  ");
}

void lc_tr_print(FILE *out, struct lc_tr *tr, size_t indent)
{
 if (tr == NULL)
 {
  print_indent(out, indent);
  fprintf(out, "NULL\n");
  return;
 }

 switch (tr->type)
 {
 case LC_REF:
  print_indent(out, indent);
  fprintf(out, "| REF: %.*s %p %zu\n", (int)tr->ref.length, tr->ref.start, tr, tr->ref_count);
  break;
 case LC_APP:
  print_indent(out, indent);
  fprintf(out, "| APP: %p %zu\n", tr, tr->ref_count);
  lc_tr_print(out, tr->app.fn, indent + 1);
  lc_tr_print(out, tr->app.arg, indent + 1);
  break;
 case LC_ABS:
  print_indent(out, indent);
  fprintf(out, "| ABS: %p %zu\n", tr, tr->ref_count);
  lc_tr_print(out, tr->abs.arg, indent + 1);
  lc_tr_print(out, tr->abs.body, indent + 1);
  break;
 case LC_THUNK:
  print_indent(out, indent);
  fprintf(out, "| THUNK: %p %zu\n", tr, tr->ref_count);
  if (tr->thunk.eval != NULL)
  {
   lc_tr_print(out, tr->thunk.eval, indent + 1);
  }
  else
  {
   lc_tr_print(out, tr->thunk.val, indent + 1);
  }
  break;
 case LC_C_VALUE:
  print_indent(out, indent);
  fprintf(out, "| C_VALUE: %p %zu\n", tr, tr->ref_count);
  break;
 case LC_C_FUNC:
  print_indent(out, indent);
  fprintf(out, "| C_FUNC: %p %zu\n", tr, tr->ref_count);
  break;
 default:
  print_indent(out, indent);
  fprintf(out, "| ?? unkown tree type ??\r\n");
 }
}

int take_space(char **str)
{
 while (isspace(**str))
  (*str)++;
 return 1;
}

int take_char(char **str, char ch)
{
 if (**str == ch)
 {
  (*str)++;
  return 1;
 }

 return 0;
}

struct lc_tr *parse_lc(char **str);
struct lc_tr *parse_lc_abs(char **str);
struct lc_tr *parse_lc_app(char **str);
struct lc_tr *parse_lc_term(char **str);
struct lc_tr *parse_lc_ref(char **str);

static char *to_lc_string(char *start, size_t length)
{
 char *ret = malloc(length + 1);

 char *cur = ret;
 for (size_t i = 0; i < length; i++)
 {
  if (start[i] == '\\' && (start[i + 1] == '"' || start[i + 1] == '\\'))
   *(cur++) = start[i + 1];
  else
   *(cur++) = start[i];
 }

 *cur = 0;
 return ret;
}

// TODO less gross?
struct lc_tr *parse_lc_ref(char **str)
{
 char *start = *str;

 if (**str == '"')
 {
  (*str)++;
  while (**str && **str != '"')
  {
   if (**str == '\\' && ((*str)[1] == '"' || (*str)[1] == '\\'))
    (*str)++;
   (*str)++;
  }

  if (**str == 0)
  {
   *str = start;
  }
  else
  {
   (*str)++;
   return lc_tr_c_value(to_lc_string(start + 1, *str - start - 2));
  }
 }

 while (isgraph(**str) && !isspace(**str) && **str != '(' && **str != ')' && **str != '.' && **str != '\\')
 {
  (*str)++;
 }

 if (start != *str)
 {
  return lc_tr_ref(start, *str - start);
 }

 *str = start;
 return NULL;
}

struct lc_tr *parse_lc_term(char **str)
{
 char *start = *str;

 struct lc_tr *expr = NULL;

 if (take_char(str, '(') && take_space(str) && (expr = parse_lc(str)) && take_space(str) && take_char(str, ')'))
 {
  return expr;
 }

 lc_tr_free(expr);
 expr = NULL;
 *str = start;

 return parse_lc_ref(str);
}

struct lc_tr *parse_lc_app(char **str)
{
 char *start = *str;
 struct lc_tr *term = NULL;
 if ((term = parse_lc_term(str)))
 {
  struct lc_tr *next_term = NULL;
  while (take_space(str) && (next_term = parse_lc_term(str)))
  {
   term = lc_tr_app(term, next_term);
  }

  return term;
 }

 lc_tr_free(term);
 *str = start;

 return NULL;
};

struct lc_tr *parse_lc_abs(char **str)
{
 char *start = *str;

 struct lc_tr *arg = NULL, *body = NULL;
 ;

 if (take_char(str, '\\') && take_space(str) && (arg = parse_lc_ref(str)) && take_space(str) && take_char(str, '.') && take_space(str) && (body = parse_lc(str)))
 {

  return lc_tr_abs(arg, body);
 }

 lc_tr_free(arg);
 arg = NULL;
 lc_tr_free(body);
 body = NULL;
 *str = start;

 return NULL;
}

struct lc_tr *parse_lc(char **str)
{

 char *start = *str;

 struct lc_tr *fn = NULL, *arg = NULL;
 if ((fn = parse_lc_app(str)) && take_space(str) && (arg = parse_lc_abs(str)))
 {
  return lc_tr_app(fn, arg);
 }

 lc_tr_free(fn);
 fn = NULL;
 lc_tr_free(arg);
 arg = NULL;
 *str = start;

 if ((fn = parse_lc_app(str)))
  return fn;

 lc_tr_free(fn);
 fn = NULL;
 return parse_lc_abs(str);
}

struct lc_tr *parse(char *str)
{
 take_space(&str);
 return parse_lc(&str);
}

int lc_tr_ref_eq(struct lc_tr *a, struct lc_tr *b)
{
 if (a->type == LC_REF && b->type == LC_REF)
 {
  if (a->ref.length == b->ref.length)
  {
   return strncmp(a->ref.start, b->ref.start, a->ref.length) == 0;
  }
 }
 return 0;
}

struct lc_env
{
 struct lc_tr *ref;
 struct lc_tr *val;
 struct lc_env *next;
};

struct lc_env *lc_env(struct lc_tr *ref, struct lc_tr *val, struct lc_env *next)
{
 struct lc_env *ret = malloc(sizeof(struct lc_env));
 ret->ref = ref;
 ret->val = val;
 ret->next = next;
 return ret;
}

void lc_env_free(struct lc_env *env)
{
 if (env == NULL)
  return;

 lc_tr_free(env->ref);
 lc_tr_free(env->val);
 lc_env_free(env->next);
 free(env);
}

struct lc_tr *lc_env_lookup(struct lc_env *env, struct lc_tr *ref)
{
 if (env == NULL)
 {
  ref->ref_count++;
  return ref;
 }

 if (lc_tr_ref_eq(ref, env->ref))
 {
  env->val->ref_count++;
  return env->val;
 }

 return lc_env_lookup(env->next, ref);
}

struct lc_tr *lc_tr_beta(struct lc_tr *body, struct lc_tr *ref, struct lc_tr *val)
{
 switch (body->type)
 {
 case LC_REF:
 {
  if (lc_tr_ref_eq(ref, body))
  {
   val->ref_count++;
   return val;
  }

  body->ref_count++;
  return body;
 }
 case LC_APP:
 {
  return lc_tr_app(lc_tr_beta(body->app.fn, ref, val), lc_tr_beta(body->app.arg, ref, val));
 }
 case LC_ABS:
 {
  if (lc_tr_ref_eq(ref, body->abs.arg))
  {
   body->ref_count++;
   return body;
  }

  body->abs.arg->ref_count++;
  return lc_tr_abs(
      body->abs.arg,
      lc_tr_beta(body->abs.body, ref, val));
 }
 case LC_THUNK:
 {
  body->ref_count++;
  return body;
 }
 case LC_C_VALUE:
 {
  body->ref_count++;
  return body;
 }
 case LC_C_FUNC:
 {
  body->ref_count++;
  return body;
 }
 }

 fprintf(stderr, "\r\nreached end of lc_tr_beta\r\n");
 return NULL;
}

struct lc_tr *eval_lc(struct lc_tr *tr, struct lc_env *env);

static size_t function_applications = 0;
struct lc_tr *lc_tr_call(struct lc_tr *fn, struct lc_tr *arg, struct lc_env *env)
{

 function_applications++;

 switch (fn->type)
 {
 case LC_ABS:
 {
  arg->ref_count++;
  struct lc_tr *ret,
      *thunk = lc_tr_thunk(arg, NULL),
      *beta = lc_tr_beta(fn->abs.body, fn->abs.arg, thunk);

  ret = eval_lc(beta, env);

  lc_tr_free(thunk);
  lc_tr_free(beta);
  lc_tr_free(fn);

  return ret;
 }
 case LC_C_FUNC:
 {
  struct lc_tr *val = eval_lc(arg, env),
               *ret = fn->c_func.func(val);

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

struct lc_tr *eval_lc(struct lc_tr *tr, struct lc_env *env)
{
 switch (tr->type)
 {
 case LC_REF:
  return lc_env_lookup(env, tr);
 case LC_APP:
  return lc_tr_call(eval_lc(tr->app.fn, env), tr->app.arg, env);
 case LC_ABS:
  tr->ref_count++;
  return tr;
 case LC_THUNK:
 {
  if (tr->thunk.eval == NULL)
  {
   tr->thunk.eval = eval_lc(tr->thunk.val, env);
  }

  tr->thunk.eval->ref_count++;
  return tr->thunk.eval;
 }
 case LC_C_VALUE:
 {
  tr->ref_count++;
  return tr;
 }
 case LC_C_FUNC:
 {
  tr->ref_count++;
  return tr;
 }
 }

 fprintf(stderr, "\r\nreached end of eval_lc %p %p\r\n", tr, env);
 return NULL;
}

char *read_file(char *path)
{
 FILE *file = fopen(path, "r"); // might need "rb" for windows :skull:

 if (file == NULL)
  return NULL;

 if (fseek(file, 0, SEEK_END) != 0)
  return NULL;

 long length = ftell(file);
 if (length == -1L)
  return NULL;

 if (fseek(file, 0, SEEK_SET) != 0)
  return NULL;

 size_t cur_length = 0, target_length = (size_t)length;
 char *ret = malloc(target_length + 1);

 while (cur_length < target_length)
 {
  cur_length += fread(ret + cur_length, 1, target_length - cur_length, file);
 }

 fclose(file);
 ret[target_length] = 0;
 return ret;
}

static struct lc_tr *lc_c_print_tr(struct lc_tr *arg)
{
 lc_tr_print(stdout, arg, 0);
 arg->ref_count++;
 return arg;
}

static struct lc_tr *lc_c_print_cstr(struct lc_tr *arg)
{
 if (arg->type == LC_C_VALUE)
 {
  printf("%s", (char *)arg->c_value.value);
  arg->ref_count++;
  return arg;
 }

 fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
 arg->ref_count++;
 return arg;
}

static struct lc_tr *lc_c_print_i(struct lc_tr *arg)
{
 if (arg->type == LC_C_VALUE)
 {
  printf("\n\n%d\n\n", *(int *)arg->c_value.value);
  arg->ref_count++;
  return arg;
 }

 fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
 arg->ref_count++;
 return arg;
}

static struct lc_tr *lc_c_atoi(struct lc_tr *arg)
{
 if (arg->type == LC_C_VALUE)
 {
  int *box = malloc(sizeof(int));
  *box = atoi((char *)arg->c_value.value);
  return lc_tr_c_value(box);
 }

 fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
 arg->ref_count++;
 return arg;
}

static struct lc_tr *lc_c_2n(struct lc_tr *arg)
{
 if (arg->type == LC_C_VALUE)
 {
  int *box = malloc(sizeof(int));
  *box = *((int *)arg->c_value.value) * 2;
  return lc_tr_c_value(box);
 }

 fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
 arg->ref_count++;
 return arg;
}

static struct lc_tr *lc_c_inc(struct lc_tr *arg)
{
 if (arg->type == LC_C_VALUE)
 {
  int *box = malloc(sizeof(int));
  *box = *((int *)arg->c_value.value) + 1;
  return lc_tr_c_value(box);
 }

 fprintf(stderr, "\r\n%s: unexpected value\r\n", __func__);
 arg->ref_count++;
 return arg;
}
