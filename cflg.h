// ******                          ******
// ******                          ******
// ******   HEADER SECTION START   ******
// ******                          ******
// ******                          ******
#ifndef CFLG_H_INCLUDE
#define CFLG_H_INCLUDE

#define CFLG_IMPLEMENTATION

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// return values of parser functions
#define CFLG_PARSE_OK 0
#define CFLG_PARSE_ARG_NEEDED -1
#define CFLG_PARSE_ARG_INVALID -2
#define CFLG_PARSE_ARG_FORCED -3

// invalid option error value
#define CFLG_PARSE_OPT_INVALID -4
// a long option cannot be auto completed
#define CFLG_PARSE_OPT_AMBIGUOUS -5

#ifndef CFLG_NO_SHORT_NAMES
#define parser_context_t cflg_parser_context_t
#define flg_t cflg_flg_t
#define flgset_t cflg_flgset_t
#define flgset_parse cflg_flgset_parse
#define flgset_int cflg_flgset_int
#define flgset_uint cflg_flgset_uint
#define flgset_int64 cflg_flgset_int64
#define flgset_uint64 cflg_flgset_uint64
#define flgset_string cflg_flgset_string
#define flgset_bool cflg_flgset_bool
#define flgset_float cflg_flgset_float
#define flgset_double cflg_flgset_double
#define flgset_func cflg_flgset_func
#define PARSE_ARG_NEEDED CFLG_PARSE_ARG_NEEDED
#define PARSE_ARG_INVALID CFLG_PARSE_ARG_INVALID
#endif

typedef struct {
  bool has_been_parsed;
  void *dest;
  const char *arg; // if flags needed an argument
} cflg_parser_context_t;

typedef struct cflg_flg {
  int (*parser)(cflg_parser_context_t *ctx);
  void *dest;
  const char *usage;
  const char *arg_name;
  const char *name_long;
  char name;
  bool has_seen;
  struct cflg_flg *next;
} cflg_flg_t;

typedef struct {
  bool parsed;
  int narg;    // number of arguments remaining after flags have been processed.
  char **args; // non-flag arguments after flags have beeen processed.
  const char *prog_name; // name of the program
  cflg_flg_t *flgs;
} cflg_flgset_t;

#define CFLG_FALLBACK(s, def) ((s) ? (s) : (def))

// cflg_new_flag has been implemented using c99 compound literals
#define cflg_new_flag(flgset, parse_function, var, opt, opt_long, arg, desc)   \
  (flgset)->flgs = &(cflg_flg_t){.name = (opt),                                \
                                 .name_long = (opt_long),                      \
                                 .parser = (parse_function),                   \
                                 .dest = (var),                                \
                                 .usage = (desc),                              \
                                 .arg_name = (arg),                            \
                                 .next = (flgset)->flgs};

#define cflg_flgset_int(flgset, p, name, name_long, arg_name, usage)           \
  cflg_new_flag((flgset), (cflg_parse_int), (int *)(p), (name), (name_long),   \
                CFLG_FALLBACK((arg_name), "int"), (usage))

#define cflg_flgset_bool(flgset, p, name, name_long, usage)                    \
  cflg_new_flag((flgset), (cflg_parse_bool), (bool *)(p), (name), (name_long), \
                NULL, (usage))

#define cflg_flgset_string(flgset, p, name, name_long, arg_name, usage)        \
  cflg_new_flag((flgset), (cflg_parse_string), (char **)(p), (name),           \
                (name_long), CFLG_FALLBACK((arg_name), "string"), (usage))

#define cflg_flgset_float(flgset, p, name, name_long, arg_name, usage)         \
  cflg_new_flag((flgset), (cflg_parse_float), (float *)(p), (name),            \
                (name_long), CFLG_FALLBACK((arg_name), "float"), (usage));

#define cflg_flgset_uint(flgset, p, name, name_long, arg_name, usage)          \
  cflg_new_flag((flgset), (cflg_parse_uint), (uint *)(p), (name), (name_long), \
                CFLG_FALLBACK((arg_name), "uint"), (usage));

#define cflg_flgset_int64(flgset, p, name, name_long, arg_name, usage)         \
  cflg_new_flag((flgset), (cflg_parse_int64), (int64_t *)(p), (name),          \
                (name_long), CFLG_FALLBACK((arg_name), "int64"), (usage))

#define cflg_flgset_uint64(flgset, p, name, name_long, arg_name, usage)        \
  cflg_new_flag((flgset), (cflg_parse_uint64), (uint64_t *)(p), (name),        \
                (name_long), CFLG_FALLBACK((arg_name), "uint64"), (usage))

#define cflg_flgset_double(flgset, p, name, name_long, arg_name, usage)        \
  cflg_new_flag((flgset), (cflg_parse_double), (double *)(p), (name),          \
                (name_long), CFLG_FALLBACK((arg_name), "double"), (usage));

#define cflg_flgset_func(flgset, p, name, name_long, arg_name, usage, parser)  \
  cflg_new_flag((flgset), (parser), (p), (name), (name_long), (arg_name),      \
                (usage))

int cflg_flgset_parse(cflg_flgset_t *flgset, int argc, char *argv[]);

// default parser functions
int cflg_parse_bool(cflg_parser_context_t *ctx);
int cflg_parse_int(cflg_parser_context_t *ctx);
int cflg_parse_uint(cflg_parser_context_t *ctx);
int cflg_parse_int64(cflg_parser_context_t *ctx);
int cflg_parse_uint64(cflg_parser_context_t *ctx);
int cflg_parse_float(cflg_parser_context_t *ctx);
int cflg_parse_double(cflg_parser_context_t *ctx);
int cflg_parse_string(cflg_parser_context_t *ctx);

// ******                        ******
// ******                        ******
// ******   HEADER SECTION END   ******
// ******                        ******
// ******                        ******
//
#endif // CFLG_H_INCLUDE

#ifdef CFLG_IMPLEMENTATION
//    ******                                  ******
//    ******                                  ******
//    ******   IMPLEMENTATION SECTION START   ******
//    ******                                  ******
//    ******                                  ******

#define CFLG_STRLEN(s) ((s) ? (strlen(s)) : (0))
#define CFLG_ISEMPTY(s) (((s) == NULL) || (*(s) == '\0'))
#define CFLG_STRNCMP(s1, s2, n) ((s1 && s2) ? (strncmp(s1, s2, n)) : (-1))
#define CFLG_NEED_VALUE(flg) ((flg)->arg_name != NULL)

#define CFLG_ISHELP(f, l)                                                      \
  (l == 1 && f[0] == 'h') || (l == strlen("help") && !memcmp(f, "help", l))

#define CFLG_FOREACH(item, flgs)                                               \
  for (cflg_flg_t *item = (flgs); item != NULL; item = item->next)

void cflg_print_flags(cflg_flg_t *flags);
void cflg_print_err(int err_code, const char *prog_name, bool is_short,
                    const char *opt, size_t opt_len, const char *arg);

#ifdef CFLG_DEBUG
#define debug(fmt, args...)                                                    \
  fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__,     \
          ##args)
#else
#define debug(fmt, args...) /* Don't do anything in release builds */
#endif

const char *cflg_find_base(const char *path) {
  for (int i = strlen(path) - 1; i >= 0; --i) {
    if (path[i] == '/' || path[i] == '\\') {
      debug("/ or \\ found at %d in %s\n", i, path);
      return path + i + 1;
    }
  }
  return path;
}

int cflg_flg_find(cflg_flg_t *flgs, const char *opt, int opt_len,
                  cflg_flg_t **res) {
  *res = NULL;
  bool ambiguous = false;

  CFLG_FOREACH(i, flgs) {
    if (!CFLG_STRNCMP(i->name_long, opt, opt_len)) {
      size_t name_len = CFLG_STRLEN(i->name_long);

      // if it's an exact match
      if (opt_len == name_len) {
        *res = i;
        return CFLG_PARSE_OK;
      }
      // if it's a partial match
      if (opt_len < name_len) {
        // if there were other partial matches before
        if (*res) {
          //  if an exact match not found later, returns an error
          ambiguous = true;
        }
        *res = i;
      }
    }
  }

  if (ambiguous) {
    return CFLG_PARSE_OPT_AMBIGUOUS;
  }

  if (*res) {
    return CFLG_PARSE_OK;
  }

  return CFLG_PARSE_OPT_INVALID;
}

int cflg_flgset_parse(cflg_flgset_t *fset, int argc, char *argv[]) {

  if (fset->parsed)
    return 0;

  // order:
  // 1. PROGRAM_NAME macro
  // 2. explicit name
  // 3. use executable name
#ifndef PROGRAM_NAME
  if (fset->prog_name == NULL)
    fset->prog_name = cflg_find_base(argv[0]);
#else
  fset->prog_name = PROGRAM_NAME;
#endif
  debug("prog_name set to %s\n", fset->prog_name);

  fset->narg = 0;
  fset->args = (char **)malloc(argc * sizeof(char *));
  if (!fset->args) {
    return -1;
  }

  int i = 0;
  for (; i < argc; ++i) {

    int len = strlen(argv[i]);

    // if not a flag, add it to args and continue
    if (len < 2 || argv[i][0] != '-') {
      fset->args[fset->narg] = argv[i];
      fset->narg++;
      continue;
    }

    cflg_parser_context_t ctx = {0};

    // if it's a long flag
    if (argv[i][1] == '-') {
      if (len == 2)
        break; // -- => stop parsing flags
      debug("long flag is detected\n");

      // search for '=' in flag
      char *arg = strchr(argv[i], '=');
      bool arg_forced = false;
      const char *flag = argv[i] + 2;
      size_t flag_len;
      if (arg == NULL) /* no '=' in flag */ {
        arg = argv[i + 1];
        flag_len = len - 2;
      } else {
        arg_forced = true;
        flag_len = arg - flag;
        arg++;
      }

      // if flag is "--help" print help
      if (CFLG_ISHELP(flag, flag_len)) {
        if (arg_forced) {
          debug("argument forced on --help option\n");
          cflg_print_err(CFLG_PARSE_ARG_FORCED, fset->prog_name, false, flag,
                         flag_len, arg);
        }
        debug("--help detected printing flags and "
              "exiting\n");
        cflg_print_flags(fset->flgs);
        exit(0);
      }

      debug("looking up the list for a matching flag\n");
      cflg_flg_t *f = NULL;
      int res = cflg_flg_find(fset->flgs, flag, flag_len, &f);
      if (res != CFLG_PARSE_OK) {
        cflg_print_err(res, fset->prog_name, false, flag, flag_len, arg);
      }

      if (!CFLG_NEED_VALUE(f) && arg_forced) {
        cflg_print_err(CFLG_PARSE_ARG_FORCED, fset->prog_name, false, flag,
                       flag_len, arg);
      }

      if (CFLG_NEED_VALUE(f)) {
        if (!arg_forced) {
          i++;
        }
        if (CFLG_ISEMPTY(arg)) {
          cflg_print_err(CFLG_PARSE_ARG_NEEDED, fset->prog_name, false, flag,
                         flag_len, arg);
        }
        ctx.arg = arg;
      }
      ctx.has_been_parsed = f->has_seen;
      ctx.dest = f->dest;

      res = f->parser(&ctx);
      if (res != CFLG_PARSE_OK) {
        cflg_print_err(res, fset->prog_name, false, flag, flag_len, arg);
      }

      f->has_seen = true;
    }
    // if it's a short flag
    else {
      // iterate over all short flags
      for (int j = 1; j < len; ++j) {
        char *flag = argv[i] + j;
        if (CFLG_ISHELP(flag, 1)) {
          cflg_print_flags(fset->flgs);
          exit(0);
        }
        cflg_flg_t *f = NULL;
        CFLG_FOREACH(i, fset->flgs) {
          if (i->name == *flag) {
            f = i;
            break;
          }
        }
        if (f == NULL) {
          cflg_print_err(CFLG_PARSE_OPT_INVALID, fset->prog_name, true, flag, 1,
                         NULL);
        }
        bool break_loop = false;
        char *arg = argv[i] + j + 1;
        if (CFLG_NEED_VALUE(f)) {
          if (CFLG_ISEMPTY(arg)) {
            arg = argv[i + 1];
          }
          if (CFLG_ISEMPTY(arg)) {
            cflg_print_err(CFLG_PARSE_ARG_NEEDED, fset->prog_name, false, flag,
                           1, arg);
          }
          i++;
          break_loop = true;
          ctx.arg = arg;
        }
        ctx.dest = f->dest;
        ctx.has_been_parsed = f->has_seen;

        int res = f->parser(&ctx);
        if (res != CFLG_PARSE_OK) {
          cflg_print_err(res, fset->prog_name, true, flag, 1, arg);
        }

        f->has_seen = true;
        if (break_loop)
          break;
      }
    }
  }

  for (i = i + 1; i < argc; ++i) {
    fset->args[fset->narg] = argv[i];
    fset->narg++;
  }

  fset->parsed = true;
  return 0;
}

void parseflg(cflg_flgset_t *flgset, char *first, char *second) {
  for (int i = 1; i < strlen(first); ++i) {
  }
}

int cflg_parse_bool(cflg_parser_context_t *ctx) {
  if (!ctx->has_been_parsed) {
    *(bool *)ctx->dest = !(*(bool *)ctx->dest);
  }
  return CFLG_PARSE_OK;
}

int cflg_parse_int(cflg_parser_context_t *ctx) {

  char *endptr;
  int n = strtol(ctx->arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(int *)ctx->dest = n;

  return CFLG_PARSE_OK;
}

int cflg_parse_uint(cflg_parser_context_t *ctx) {

  char *endptr;
  uint n = strtoul(ctx->arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(uint *)ctx->dest = n;

  return CFLG_PARSE_OK;
}

int cflg_parse_int64(cflg_parser_context_t *ctx) {

  char *endptr;
  int64_t n = strtoll(ctx->arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(int64_t *)ctx->dest = n;

  return CFLG_PARSE_OK;
}

int cflg_parse_uint64(cflg_parser_context_t *ctx) {

  char *endptr;
  uint64_t n = strtoull(ctx->arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(uint64_t *)ctx->dest = n;

  return CFLG_PARSE_OK;
}

int cflg_parse_float(cflg_parser_context_t *ctx) {

  char *endptr;
  float n = strtof(ctx->arg, &endptr);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(float *)ctx->dest = n;

  return CFLG_PARSE_OK;
}

int cflg_parse_double(cflg_parser_context_t *ctx) {

  char *endptr;
  double n = strtod(ctx->arg, &endptr);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(double *)ctx->dest = n;

  return CFLG_PARSE_OK;
}

int cflg_parse_string(cflg_parser_context_t *ctx) {
  *(char **)ctx->dest = (char *)ctx->arg;
  return CFLG_PARSE_OK;
}

void cflg_print_flags(cflg_flg_t *flags) {
  // find the maximum length
  int max_width = 0;
  CFLG_FOREACH(i, flags) {
    int curr_len = CFLG_STRLEN(i->name_long) + CFLG_STRLEN(i->arg_name);
    if (curr_len > max_width) {
      max_width = curr_len;
    }
  }
  max_width += 15;
  debug("maximum len is %d\n", max_width);

  // print flags
  CFLG_FOREACH(f, flags) {
    int current_len = 0;

    // [1] name, name_long=<arg_name> usage
    // [2] name  <arg_name>           usage
    // [3]       name_long=<arg_name> usage
    current_len += printf("  ");

    if (f->name) {
      current_len += printf("-%c", f->name);
    } else {
      current_len += printf("  ");
    }

    if (f->name && f->name_long) {
      current_len += printf(", ");
    } else {
      current_len += printf("  ");
    }

    if (f->name_long) {
      current_len += printf("--%s", f->name_long);
    }

    if (f->arg_name) {
      if (f->name_long) {
        current_len += printf("=");
      }
      current_len += printf("%s", f->arg_name);
    }

    if (max_width > current_len) {
      printf("%*s", max_width - current_len, "");
    }

    if (f->usage) {
      printf("%s\n", f->usage);
    }
  }
}

void cflg_print_err(int err_code, const char *prog_name, bool is_short,
                    const char *opt, size_t opt_len, const char *arg) {

  // TODO: gnu seems to print different error message
  // base on short or long format is it really
  // necessary in this library?
  //
  //
  fprintf(stderr, "%s: ", prog_name);

  const char *invalid_opt_err, *invalid_arg_err, *need_arg_err, *forced_arg_err,
      *ambiguous_opt_err;
  if (is_short) {
    invalid_opt_err = "invalid option -- '%.*s'\n";
    invalid_arg_err = "invalid '%.*s' argument: '%s'\n";
    need_arg_err = "option requires an argument -- '%.*s'\n";
  } else {
    invalid_opt_err = "unrecognize option '--%.*s'\n";
    invalid_arg_err = "invalid --%.*s argument: '%s'\n";
    need_arg_err = "option '--%.*s' requires an argument\n";
    forced_arg_err = "option '--%.*s' doesn't allow an argument\n";
    ambiguous_opt_err = "option '--%.*s' is ambiguous\n";
  }

  switch (err_code) {

  case CFLG_PARSE_OPT_INVALID:
    fprintf(stderr, invalid_opt_err, opt_len, opt);
    break;

  case CFLG_PARSE_ARG_INVALID:
    fprintf(stderr, invalid_arg_err, opt_len, opt, arg);
    break;

  case CFLG_PARSE_ARG_NEEDED:
    fprintf(stderr, need_arg_err, opt_len, opt);
    break;

  case CFLG_PARSE_ARG_FORCED:
    fprintf(stderr, forced_arg_err, opt_len, opt);
    break;

  case CFLG_PARSE_OPT_AMBIGUOUS:
    fprintf(stderr, ambiguous_opt_err, opt_len, opt);
    break;
  }

  fprintf(stderr, "Try '%s --help' for more information.\n", prog_name);
  exit(1);
}

// ******                                  ******
// ******                                  ******
// ******   IMPLEMENTATION SECTION END     ******
// ******                                  ******
// ******                                  ******
#endif // IMPLEMENTATION
