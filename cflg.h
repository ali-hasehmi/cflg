//
// ******                          ******
// ******                          ******
// ******   HEADER SECTION START   ******
// ******                          ******
// ******                          ******
//
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
#define OK CFLG_OK
#define OK_NO_ARG CFLG_OK_NO_ARG
#define ERR_ARG_NEEDED CFLG_ERR_ARG_NEEDED
#define ERR_ARG_INVALID CFLG_ERR_ARG_INVALID
#endif

// stores the state of each command-line option while parsing
typedef struct {
  const char *opt;      // points the current command-line option, it might not be null terminated, always check opt_len.
  size_t opt_len;       // stores the length of the current command-line option.
  bool is_opt_short;    // stores whether the option is short(e.g. -v) or long(e.g. --verbose, --v, --ver).
  bool has_been_parsed; // stores whether the corresponding flag was used before
  bool is_arg_forced;   // is option's argument forced (e.g. '--verbose=3' ).
  void *dest;           // points to the user specified memory
  const char *arg;      // points to the option's argument which is always null terminated, NULL if not provided. 
} cflg_parser_context_t;

/* parser function type
   Function pointer type for parsing a command-line flag's argument.
   Takes a cflg_parser_context_t* containing flag details and argument,
   returns a status code (e.g., CFLG_OK, CFLG_ERR_ARG_NEEDED). */
typedef int (*cflg_parser_t)(cflg_parser_context_t *);

/* Return values for command-line flag parser functions */
#define CFLG_OK 0               /* Successfully parsed option and consumed its argument */
#define CFLG_OK_NO_ARG 1        /* Successfully parsed option; no argument was required or taken */
#define CFLG_ERR_ARG_NEEDED -1  /* Parsing failed: option requires an argument, but none was provided */
#define CFLG_ERR_ARG_INVALID -2 /* Parsing failed: option's argument was provided but invalid (e.g., wrong format) */
#define CFLG_ERR_ARG_FORCED -3  /* Parsing failed: option's argument was forced but not needed */


typedef struct cflg_flg cflg_flg_t;
struct cflg_flg {
  cflg_parser_t parser;
  void *dest;
  const char *usage;
  const char *arg_name;
  const char *name_long;
  char name;
  bool has_seen;
  cflg_flg_t *next;
};

typedef struct cflg_flgset cflg_flgset_t;

/* Function pointer type for handling help requests when parsing '-h' or '--help'.
   Called with a cflg_flgset_t* containing flag definitions to display help output. */
typedef void (*cflg_usage_t)(cflg_flgset_t *);

struct cflg_flgset {
  bool parsed;
  int narg;    // Number of non-flag arguments in args, including argv[0]
  char **args; // Pointer to argv used for parsing
  const char *prog_name; // name of the program
  cflg_flg_t *flgs;
  cflg_usage_t usage; // if not specified, falls back to default usage function (see cflg_print_help_)
};

#define CFLG_FALLBACK(s, def) ((s) ? (s) : (def))

// cflg_new_flag has been implemented using c99 compound literals
#define cflg_new_flag(flgset, parse_function, var, opt, opt_long, arg, desc)   \
  ((flgset)->flgs = &(cflg_flg_t) {                                             \
                    .name = (opt), \
                    .name_long = (opt_long), \
                    .parser = (parse_function),\
                    .dest = (var),\
                    .usage = (desc), \
                    .arg_name = (arg), \
                    .next = (flgset)->flgs  \
                  })

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

//
// ******                        ******
// ******                        ******
// ******   HEADER SECTION END   ******
// ******                        ******
// ******                        ******
//
#endif // CFLG_H_INCLUDE

#ifdef CFLG_IMPLEMENTATION
//
// ******                                  ******
// ******                                  ******
// ******   IMPLEMENTATION SECTION START   ******
// ******                                  ******
// ******                                  ******
//

// invalid option error value
#define CFLG_ERR_OPT_INVALID -4
// a long option cannot be auto completed
#define CFLG_ERR_OPT_AMBIGUOUS -5

#define CFLG_STRLEN(s) ((s) ? (strlen(s)) : (0))
#define CFLG_ISEMPTY(s) (((s) == NULL) || (*(s) == '\0'))
#define CFLG_STRNCMP(s1, s2, n) ((s1 && s2) ? (strncmp(s1, s2, n)) : (-1))

#define CFLG_ISHELP(f) ((f) == 'h') 
#define CFLG_ISHELP_LONG(f,l) ((l) == strlen("help") && !memcmp((f), "help", l))

#define CFLG_FOREACH(item, flgs)                                               \
  for (cflg_flg_t *item = (flgs); item != NULL; item = item->next)

void cflg_print_flags(cflg_flg_t *flags);
void cflg_print_err(int err_code, cflg_flgset_t *fs, cflg_parser_context_t* ctx);


// default print usage function
// prints usage string and all flag names (short and long)
void cflg_print_help_(cflg_flgset_t *fset) {
    printf("Usage: %s [OPTION]... [COMMAND]...\n\n", fset->prog_name);
    cflg_print_flags(fset->flgs);
}

int cflg_parse_help(cflg_parser_context_t *ctx) {
    if (ctx->is_arg_forced) {
        return CFLG_ERR_ARG_FORCED;
    }
    cflg_flgset_t* fset = (cflg_flgset_t*) ctx->dest;
    fset->usage(fset);
    return OK_NO_ARG;
}

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
        return CFLG_OK;
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
    return CFLG_ERR_OPT_AMBIGUOUS;
  }

  if (*res) {
    return CFLG_OK;
  }

  return CFLG_ERR_OPT_INVALID;
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
  fset->args = argv;

  // if user didn't provide usage function, fall back to default 
  if (fset->usage == NULL) {
        fset->usage = cflg_print_help_;
  }

  cflg_new_flag(fset, cflg_parse_help, fset, 'h', "help", NULL, "print this help");

  int last_nonopt = 0;
  int i = 1; // argv[0] is always assumed to be the name of the executable
  for (; i < argc; ++i) {

    int len = strlen(argv[i]);

    // if not a flag, add it to args and continue
    if (len < 2 || argv[i][0] != '-') {
      last_nonopt++;
      char *tmp = argv[i];
      argv[i] = argv[last_nonopt];
      argv[last_nonopt] = tmp;
      continue;
    }

    cflg_parser_context_t ctx = {0};

    // if it's a long flag
    if (argv[i][1] == '-') {
      if (len == 2) {
        break; // -- => stop parsing flags
      }
      debug("long flag is detected\n");

      ctx.is_opt_short = false;

      // search for '=' in flag
      ctx.arg = strchr(argv[i], '=');
      ctx.is_arg_forced = false;
      ctx.opt = argv[i] + 2;
      if (ctx.arg == NULL) { /* no '=' in flag */
        ctx.arg = argv[i + 1];
        ctx.opt_len = len - 2;
      } else {
        ctx.is_arg_forced = true;
        ctx.opt_len = ctx.arg - ctx.opt;
        ctx.arg++;
      }

      debug("looking up the list for a matching flag\n");
      cflg_flg_t *f = NULL;
      int res = cflg_flg_find(fset->flgs, ctx.opt, ctx.opt_len, &f);
      if (res != CFLG_OK) {
        cflg_print_err(res, fset, &ctx);
      }

      if (CFLG_ISEMPTY(ctx.arg)) {
        ctx.arg = NULL;
      }
      ctx.has_been_parsed = f->has_seen;
      ctx.dest = f->dest;

      res = f->parser(&ctx);
      switch (res) {
      case CFLG_OK:
        if (!ctx.is_arg_forced) {
          i++;
        }
        break;

      case CFLG_OK_NO_ARG:
        if (ctx.is_arg_forced) {
          cflg_print_err(CFLG_ERR_ARG_FORCED, fset, &ctx); 
        }
        break;

      default:
        cflg_print_err(res, fset, &ctx);
      }

      f->has_seen = true;

      if (CFLG_ISHELP_LONG(f->name_long, CFLG_STRLEN(f->name_long))) {
            debug("'--help' was requested, exiting program...\n");
            exit(0);
      }
    }
    // if it's a short flag
    else {
      // iterate over all short flags
      for (int j = 1; j < len; ++j) {
        ctx.is_opt_short = true; 

        ctx.opt = argv[i] + j;
        ctx.opt_len = 1;

        cflg_flg_t *f = NULL;
        CFLG_FOREACH(i, fset->flgs) {
          if (i->name == *ctx.opt) {
            f = i;
            break;
          }
        }
        if (f == NULL) {
          cflg_print_err(CFLG_ERR_OPT_INVALID, fset, &ctx); 
        }

        bool break_loop = false;
        ctx.arg = argv[i] + j + 1;
        if (CFLG_ISEMPTY(ctx.arg)) {
          ctx.arg = argv[i + 1];
          break_loop = true;
        }

        ctx.dest = f->dest;
        ctx.has_been_parsed = f->has_seen;

        int res = f->parser(&ctx);
        switch (res) {

        case CFLG_OK:
            if (break_loop) {
                i++;
            }
            break_loop = true;
          break;

        case CFLG_OK_NO_ARG:
          break_loop = false;
          break;

        default:
          cflg_print_err(res, fset, &ctx);
        }

        f->has_seen = true;

        if (CFLG_ISHELP(f->name)) {
            debug("'-h' was requested, exiting program...\n");
            exit(0);
        }
        if (break_loop) {
          break;
        }
      }
    }
  }

  for (i = i + 1; i < argc; ++i) {
    last_nonopt++;
    char *tmp = argv[i];
    argv[i] = argv[last_nonopt];
    argv[last_nonopt] = tmp;
  }

  // remove help flag, because after returning from this function
  // it will be deallocated
  fset->flgs = fset->flgs->next;
  fset->narg = last_nonopt + 1;
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
  return CFLG_OK_NO_ARG;
}

int cflg_parse_int(cflg_parser_context_t *ctx) {
  if (ctx->arg == NULL) {
     return CFLG_ERR_ARG_NEEDED;
  }
  char *endptr;
  int n = strtol(ctx->arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_ERR_ARG_INVALID;
  }

  *(int *)ctx->dest = n;

  return CFLG_OK;
}

int cflg_parse_uint(cflg_parser_context_t *ctx) {
  if (ctx->arg == NULL) {
    return CFLG_ERR_ARG_NEEDED;
  }
  char *endptr;
  uint n = strtoul(ctx->arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_ERR_ARG_INVALID;
  }

  *(uint *)ctx->dest = n;

  return CFLG_OK;
}

int cflg_parse_int64(cflg_parser_context_t *ctx) {
  if (ctx->arg == NULL) {
     return CFLG_ERR_ARG_NEEDED;
  }

  char *endptr;
  int64_t n = strtoll(ctx->arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_ERR_ARG_INVALID;
  }

  *(int64_t *)ctx->dest = n;

  return CFLG_OK;
}

int cflg_parse_uint64(cflg_parser_context_t *ctx) {
  if (ctx->arg == NULL) {
    return CFLG_ERR_ARG_NEEDED;
  }
  char *endptr;
  uint64_t n = strtoull(ctx->arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_ERR_ARG_INVALID;
  }

  *(uint64_t *)ctx->dest = n;

  return CFLG_OK;
}

int cflg_parse_float(cflg_parser_context_t *ctx) {
  if (ctx->arg == NULL) {
    return CFLG_ERR_ARG_NEEDED;
  }
  char *endptr;
  float n = strtof(ctx->arg, &endptr);
  if (*endptr != '\0') {
    return CFLG_ERR_ARG_INVALID;
  }

  *(float *)ctx->dest = n;

  return CFLG_OK;
}

int cflg_parse_double(cflg_parser_context_t *ctx) {
  if (ctx->arg == NULL) {
    return CFLG_ERR_ARG_NEEDED;
  }
  char *endptr;
  double n = strtod(ctx->arg, &endptr);
  if (*endptr != '\0') {
    return CFLG_ERR_ARG_INVALID;
  }

  *(double *)ctx->dest = n;

  return CFLG_OK;
}

int cflg_parse_string(cflg_parser_context_t *ctx) {
  if (ctx->arg == NULL) {
    return CFLG_ERR_ARG_NEEDED;
  }
  *(char **)ctx->dest = (char *)ctx->arg;
  return CFLG_OK;
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

    if (f->name && !CFLG_ISEMPTY(f->name_long)) {
      current_len += printf(",");
    } else {
      current_len += printf(" ");
    }

    if (!CFLG_ISEMPTY(f->name_long)) {
      current_len += printf(" --%s", f->name_long);
    }

    if (!CFLG_ISEMPTY(f->arg_name)) {
      if (!CFLG_ISEMPTY(f->name_long)) {
        current_len += printf("=");
      }
      current_len += printf("%s", f->arg_name);
    }

    if (max_width > current_len) {
      printf("%*s", max_width - current_len, "");
    }

    if (!CFLG_ISEMPTY(f->usage)) {
      printf("%s", f->usage);
    }
    printf("\n");
  }
}

void cflg_print_err(int err_code, cflg_flgset_t *fs, cflg_parser_context_t* ctx) {

  // TODO: gnu seems to print different error message
  // base on short or long format is it really
  // necessary in this library?
  //
  //
  fprintf(stderr, "%s: ", fs->prog_name);

  const char *invalid_opt_err, *invalid_arg_err, *need_arg_err, *forced_arg_err,
      *ambiguous_opt_err;
  if (ctx->is_opt_short) {
    invalid_opt_err = "invalid option -- '%.*s'";
    invalid_arg_err = "invalid '%.*s' argument: '%s'";
    need_arg_err = "option requires an argument -- '%.*s'";
  } else {
    invalid_opt_err = "unrecognize option '--%.*s'";
    invalid_arg_err = "invalid --%.*s argument: '%s'";
    need_arg_err = "option '--%.*s' requires an argument";
    forced_arg_err = "option '--%.*s' doesn't allow an argument";
    ambiguous_opt_err = "option '--%.*s' is ambiguous;";
  }

  switch (err_code) {

  case CFLG_ERR_OPT_INVALID:
    fprintf(stderr, invalid_opt_err, ctx->opt_len, ctx->opt);
    break;

  case CFLG_ERR_ARG_INVALID:
    fprintf(stderr, invalid_arg_err, ctx->opt_len, ctx->opt, ctx->arg);
    break;

  case CFLG_ERR_ARG_NEEDED:
    fprintf(stderr, need_arg_err, ctx->opt_len, ctx->opt);
    break;

  case CFLG_ERR_ARG_FORCED:
    fprintf(stderr, forced_arg_err, ctx->opt_len, ctx->opt);
    break;

  case CFLG_ERR_OPT_AMBIGUOUS:
    fprintf(stderr, ambiguous_opt_err, ctx->opt_len, ctx->opt);

    // find and print all matching options
    fprintf(stderr, " possibilities:");
    CFLG_FOREACH(item, fs->flgs) {
           if (ctx->opt_len < CFLG_STRLEN(item->name_long) &&
               !CFLG_STRNCMP(item->name_long, ctx->opt, ctx->opt_len)) {
                    fprintf(stderr, " '--%s'", item->name_long);
            }
    }

    break;
  }
  fprintf(stderr, "\n");

  fprintf(stderr, "Try '%s --help' for more information.\n", fs->prog_name);
  exit(1);
}

//
// ******                                  ******
// ******                                  ******
// ******   IMPLEMENTATION SECTION END     ******
// ******                                  ******
// ******                                  ******
//
#endif // IMPLEMENTATION
