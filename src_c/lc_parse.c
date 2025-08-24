
#include "lc_parse.h"
#include "ctype.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// TODO should probably go in a io.c file...
char *read_file(char *path) {
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

  while (cur_length < target_length) {
    cur_length += fread(ret + cur_length, 1, target_length - cur_length, file);
  }

  fclose(file);
  ret[target_length] = 0;
  return ret;
}

static int take_space(char **str) {
  while (isspace(**str))
    (*str)++;
  return 1;
}

static int take_char(char **str, char ch) {
  if (**str == ch) {
    (*str)++;
    return 1;
  }

  return 0;
}

static struct lc_tr *parse_lc(char **str);
static struct lc_tr *parse_lc_abs(char **str);
static struct lc_tr *parse_lc_app(char **str);
static struct lc_tr *parse_lc_term(char **str);
static struct lc_tr *parse_lc_ref(char **str);

static char *to_lc_string(char *start, size_t length) {
  char *ret = malloc(length + 1);

  char *cur = ret;
  for (size_t i = 0; i < length; i++) {
    if (start[i] == '\\' && (start[i + 1] == '"' || start[i + 1] == '\\'))
      *(cur++) = start[i + 1];
    else
      *(cur++) = start[i];
  }

  *cur = 0;
  return ret;
}

// TODO less gross?
static struct lc_tr *parse_lc_ref(char **str) {
  char *start = *str;

  if (**str == '"') {
    (*str)++;
    while (**str && **str != '"') {
      if (**str == '\\' && ((*str)[1] == '"' || (*str)[1] == '\\'))
        (*str)++;
      (*str)++;
    }

    if (**str == 0) {
      *str = start;
    } else {
      (*str)++;
      return lc_tr_c_value(to_lc_string(start + 1, *str - start - 2));
    }
  }

  while (isgraph(**str) && !isspace(**str) && **str != '(' && **str != ')' &&
         **str != '.' && **str != '\\') {
    (*str)++;
  }

  if (start != *str) {
    return lc_tr_ref(start, *str - start);
  }

  *str = start;
  return NULL;
}

static struct lc_tr *parse_lc_term(char **str) {
  char *start = *str;

  struct lc_tr *expr = NULL;

  if (take_char(str, '(') && take_space(str) && (expr = parse_lc(str)) &&
      take_space(str) && take_char(str, ')')) {
    return expr;
  }

  lc_tr_free(expr);
  expr = NULL;
  *str = start;

  return parse_lc_ref(str);
}

static struct lc_tr *parse_lc_app(char **str) {
  char *start = *str;
  struct lc_tr *term = NULL;
  if ((term = parse_lc_term(str))) {
    struct lc_tr *next_term = NULL;
    while (take_space(str) && (next_term = parse_lc_term(str))) {
      term = lc_tr_app(term, next_term);
    }

    return term;
  }

  lc_tr_free(term);
  *str = start;

  return NULL;
};

static struct lc_tr *parse_lc_abs(char **str) {
  char *start = *str;

  struct lc_tr *arg = NULL, *body = NULL;
  ;

  if (take_char(str, '\\') && take_space(str) && (arg = parse_lc_ref(str)) &&
      take_space(str) && take_char(str, '.') && take_space(str) &&
      (body = parse_lc(str))) {

    return lc_tr_abs(arg, body);
  }

  lc_tr_free(arg);
  arg = NULL;
  lc_tr_free(body);
  body = NULL;
  *str = start;

  return NULL;
}

static struct lc_tr *parse_lc(char **str) {

  char *start = *str;

  struct lc_tr *fn = NULL, *arg = NULL;
  if ((fn = parse_lc_app(str)) && take_space(str) &&
      (arg = parse_lc_abs(str))) {
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

struct lc_tr *parse(char *str) {
  take_space(&str);
  return parse_lc(&str);
}
