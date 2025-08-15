#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CFLAG_FLAG_ARENA_INIT_CAP
#define CFLAG_FLAG_ARENA_INIT_CAP 10
#endif

typedef enum {
  Bool,
  Int,
  String,
  Float,
  UInt,
  Int64,
  UInt64,
  Double,
  UserDefined, // TODO: this option will allow user to set custom parsing
               // function
} flagtype_t;

typedef struct {
  flagtype_t type;
  void *dest;
  const char *usage;
  const char *arg_name;
  char name;
  const char *name_long;
} flag_t;

typedef struct {
  uint32_t (*hashFunc)(const char *, size_t);
  size_t len;
  size_t cap;
  flag_t **map;
} map_t;

typedef struct flag_arena {
  struct flag_arena *next;
  size_t used;
  size_t cap;
  flag_t mem[];
} flag_arena_t;

typedef struct {
  bool parsed;
  int narg;    // number of arguments remaining after flags have been processed.
  char **args; // non-flag arguments after flags have beeen processed.
  const char *prog_name; // name of the program
  flag_arena_t *arena;
} cflg_flagset_t;

void cflg_flgset_create(cflg_flagset_t *flgset);
void cflg_flgset_destroy(cflg_flagset_t *flgset);
// int  cflg_parse(int argc, char *argv[]);
void cflg_flgset_int(cflg_flagset_t *flgset, int *p, char name,
                     const char *name_long, const char *arg_name,
                     const char *usage);
void cflg_flgset_bool(cflg_flagset_t *flgset, bool *p, char name,
                      const char *name_long, const char *usage);
void cflg_flgset_string(cflg_flagset_t *flgset, char **p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage);
void cflg_flgset_float(cflg_flagset_t *flgset, float *p, char name,
                       const char *name_long, const char *arg_name,
                       const char *usage);
void cflg_flgset_uint(cflg_flagset_t *flgset, unsigned *p, char name,
                      const char *name_long, const char *arg_name,
                      const char *usage);
void cflg_flgset_int64(cflg_flagset_t *flgset, int64_t *p, char name,
                       const char *name_long, const char *arg_name,
                       const char *usage);
void cflg_flgset_uint64(cflg_flagset_t *flgset, uint64_t *p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage);

void cflg_flgset_double(cflg_flagset_t *flgset, double *p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage);

int cflg_flagset_parse(cflg_flagset_t *flgset, int argc, char *argv[]);

void map_create_from_arena(map_t *m, flag_arena_t *a);
void map_destroy(map_t *m);
bool map_insert(map_t *m, const char *k, size_t len, flag_t *v);
flag_t *map_find(map_t *m, const char *k, size_t len);

flag_arena_t *flag_arena_new();
void flag_arena_delete(flag_arena_t *arena);
flag_t *flag_arena_alloc(flag_arena_t *arena);
size_t flag_arena_len(flag_arena_t *arena);
flag_t *flag_arena_at(flag_arena_t *arena, size_t index);

#define PARSE_ARG_REMAINED 0
#define PARSE_ARG_CONSUMED 1
#define PARSE_ARG_NEEDED -1
#define PARSE_ARG_INVALID -2
#define PARSE_OPT_INVALID -3

#define STRLEN(s) ((s) ? (strlen(s)) : (0))
#define FALLBACK(s, def) ((s) ? (s) : (def))

#define ISHELP(f, l)                                                           \
  (l == 1 && f[0] == 'h') || (l == strlen("help") && !memcmp(f, "help", l))

int parse_bool(flag_t *f, const char *arg);
int parse_int(flag_t *f, const char *arg);
int parse_uint(flag_t *f, const char *arg);
int parse_int64(flag_t *f, const char *arg);
int parse_uint64(flag_t *f, const char *arg);
int parse_float(flag_t *f, const char *arg);
int parse_double(flag_t *f, const char *arg);
int parse_string(flag_t *f, const char *arg);

int (*parse_handlers[])(flag_t *, const char *arg) = {
    [Bool] = parse_bool,     [Int] = parse_int,       [UInt] = parse_uint,
    [Int64] = parse_int64,   [UInt64] = parse_uint64, [Float] = parse_float,
    [Double] = parse_double, [String] = parse_string,
};

void print_flags(flag_arena_t *flags);
void print_err(int err_code, const char *prog_name, bool is_short,
               const char *opt, size_t opt_len, const char *arg);

void parseflg_long(cflg_flagset_t *flgset, char *first, char *second);
void parseflg(cflg_flagset_t *fset, char *first, char *second);

#ifdef CFLG_DEBUG
#define debug(fmt, args...)                                                    \
  fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__,     \
          ##args)
#else
#define debug(fmt, args...) /* Don't do anything in release builds */
#endif

const char *find_base(const char *path) {
  for (int i = strlen(path) - 1; i >= 0; --i) {
    if (path[i] == '/' || path[i] == '\\') {
      debug("/ or \\ found at %d in %s\n", i, path);
      return path + i + 1;
    }
  }
  return path;
}
void cflg_flgset_create(cflg_flagset_t *flgset) {
  debug("start creating flagset\n");
  memset(flgset, 0, sizeof(cflg_flagset_t));
  flgset->arena = flag_arena_new();
  debug("end creating flagset\n");
}

void cflg_flgset_destroy(cflg_flagset_t *flgset) {
  if (flgset->args != NULL)
    free(flgset->args);
  flag_arena_delete(flgset->arena);
}

flag_t *new_flag(flag_arena_t *arena, flagtype_t ftype, void *dest, char name,
                 const char *name_long, const char *arg_name,
                 const char *usage) {
  if (name != 0 && !isalnum(name))
    return NULL;
  flag_t *f = flag_arena_alloc(arena);
  if (f) {
    memset(f, 0, sizeof(flag_t));
    f->type = ftype;
    f->name_long = name_long;
    f->name = name;
    f->usage = usage;
    f->dest = dest;
    f->arg_name = arg_name;
  }
  return f;
}

void cflg_flgset_int(cflg_flagset_t *flgset, int *p, char name,
                     const char *name_long, const char *arg_name,
                     const char *usage) {
  debug("start adding integer to flagset\n");
  flag_t *f = new_flag(flgset->arena, Int, p, name, name_long,
                       FALLBACK(arg_name, "int"), usage);
}

void cflg_flgset_uint(cflg_flagset_t *flgset, unsigned *p, char name,
                      const char *name_long, const char *arg_name,
                      const char *usage) {
  debug("start adding unsigned integer to flagset\n");
  flag_t *f = new_flag(flgset->arena, UInt, p, name, name_long,
                       FALLBACK(arg_name, "uint"), usage);
}

void cflg_flgset_int64(cflg_flagset_t *flgset, int64_t *p, char name,
                       const char *name_long, const char *arg_name,
                       const char *usage) {
  debug("start adding int64 to flagset\n");
  flag_t *f = new_flag(flgset->arena, Int64, p, name, name_long,
                       FALLBACK(arg_name, "int64"), usage);
}

void cflg_flgset_uint64(cflg_flagset_t *flgset, uint64_t *p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage) {
  debug("start adding uint64 to flagset\n");
  flag_t *f = new_flag(flgset->arena, UInt64, p, name, name_long,
                       FALLBACK(arg_name, "uint64"), usage);
}
void cflg_flgset_bool(cflg_flagset_t *flgset, bool *p, char name,
                      const char *name_long, const char *usage) {
  debug("start adding bool to flagset\n");
  flag_t *f = new_flag(flgset->arena, Bool, p, name, name_long, NULL, usage);
}

void cflg_flgset_float(cflg_flagset_t *flgset, float *p, char name,
                       const char *name_long, const char *arg_name,
                       const char *usage) {
  debug("start adding bool to flagset\n");
  flag_t *f = new_flag(flgset->arena, Float, p, name, name_long,
                       FALLBACK(arg_name, "float"), usage);
}

void cflg_flgset_double(cflg_flagset_t *flgset, double *p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage) {
  debug("start adding bool to flagset\n");
  flag_t *f = new_flag(flgset->arena, Double, p, name, name_long,
                       FALLBACK(arg_name, "double"), usage);
}

void cflg_flgset_string(cflg_flagset_t *flgset, char **p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage) {
  debug("start adding bool to flagset\n");
  flag_t *f = new_flag(flgset->arena, String, p, name, name_long,
                       FALLBACK(arg_name, "string"), usage);
}

int cflg_flagset_parse(cflg_flagset_t *fset, int argc, char *argv[]) {

  if (fset->parsed)
    return 0;

  map_t flags;
  map_create_from_arena(&flags, fset->arena);

// order:
// 1. PROGRAM_NAME macro
// 2. explicit name
// 3. use executable name
#ifndef PROGRAM_NAME
  if (fset->prog_name == NULL)
    fset->prog_name = find_base(argv[0]);
#else
  fset->prog_name = PROGRAM_NAME;
#endif
  debug("prog_name set to %s\n", fset->prog_name);

  fset->narg = 0;
  fset->args = (char **)malloc(argc * sizeof(char *));
  if (!fset->args) {
    map_destroy(&flags);
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

    // if it's a long flag
    if (argv[i][1] == '-') {
      if (len == 2)
        break; // -- => stop parsing flags
      debug("long flag is detected\n");

      // search for '=' in flag
      char *arg = strchr(argv[i], '=');
      bool skip_next = false;
      char *flag;
      size_t flag_len;
      if (arg == NULL) /* no '=' in flag */ {
        arg = argv[i + 1];
        flag = argv[i] + 2;
        flag_len = len - 2;
        skip_next = true;
      } else {
        flag = argv[i] + 2;
        flag_len = arg - flag;
        arg++;
      }

      // if flag is "--help" print help
      if (ISHELP(flag, flag_len)) {
        debug("--help detected printing flags and exiting\n");
        map_destroy(&flags);
        print_flags(fset->arena);
        exit(0);
      }

      debug("looking up the hashtable for flag\n");
      flag_t *f = map_find(&flags, flag, flag_len);
      if (f == NULL || f->name_long == NULL ||
          strncmp(f->name_long, flag, flag_len)) {
        map_destroy(&flags);
        print_err(PARSE_OPT_INVALID, fset->prog_name, false, flag, flag_len,
                  arg);
      }
      int res = parse_handlers[f->type](f, arg);
      switch (res) {
      case PARSE_ARG_CONSUMED:
        if (skip_next) {
          i++;
        }
        break;
      case PARSE_ARG_REMAINED:
        break;
      default:
        map_destroy(&flags);
        print_err(res, fset->prog_name, false, flag, flag_len, arg);
      }
    }
    // if it's a short flag
    else {
      // iterate over all short flags
      for (int j = 1; j < len; ++j) {
        char *flag = argv[i] + j;
        if (ISHELP(flag, 1)) {
          map_destroy(&flags);
          print_flags(fset->arena);
          exit(0);
        }
        flag_t *f = map_find(&flags, flag, 1);
        if (f == NULL || f->name == 0 || f->name != flag[0]) {
          map_destroy(&flags);
          print_err(PARSE_OPT_INVALID, fset->prog_name, true, flag, 1, NULL);
        }
        bool skip_next = false;
        char *arg = argv[i] + j + 1;
        if (j + 1 == len) { // TODO: OR "*arg == '\0'" which one is better?
          arg = argv[i + 1];
          skip_next = true;
        }
        int res = parse_handlers[f->type](f, arg);
        bool break_loop = false;
        switch (res) {
        case PARSE_ARG_CONSUMED:
          break_loop = true;
          if (skip_next) {
            i++;
          }
          break;
        case PARSE_ARG_REMAINED:
          break;
        default:
          map_destroy(&flags);
          print_err(res, fset->prog_name, true, flag, 1, arg);
        }
        if (break_loop)
          break;
      }
    }
  }

  for (i = i + 1; i < argc; ++i) {
    fset->args[fset->narg] = argv[i];
    fset->narg++;
  }

  map_destroy(&flags);
  fset->parsed = true;
  return 0;
}

void parseflg(cflg_flagset_t *flgset, char *first, char *second) {
  for (int i = 1; i < strlen(first); ++i) {
  }
}

uint32_t djb2_hash(const char *s, size_t cnt) {
  uint32_t hash = 5381u;
  for (size_t i = 0; i < cnt; ++i) {
    hash = ((hash << 5) + hash) + (uint8_t)s[i];
  }
  return hash;
}

void map_create_from_arena(map_t *m, flag_arena_t *a) {
  size_t arena_len = flag_arena_len(a);
  m->map = (flag_t **)malloc(2 * arena_len * sizeof(flag_t *));
  // TODO: implement proper error handling
  assert(m->map);
  memset(m->map, 0, 2 * arena_len * sizeof(flag_t *));
  m->len = 0;
  m->cap = 2 * arena_len;
  m->hashFunc = djb2_hash;

  for (int i = 0; i < arena_len; ++i) {
    flag_t *f = flag_arena_at(a, i);
    if (f->name)
      map_insert(m, &f->name, 1, f);
    if (f->name_long)
      map_insert(m, f->name_long, strlen(f->name_long), f);
  }
}

void map_destroy(map_t *m) { free(m->map); }

bool map_insert(map_t *m, const char *key, size_t len, flag_t *v) {
  debug("start adding flag to map\n");
  // panic if key is NULL
  assert(key != NULL);

  // if map is full, resize the backing array
  if (m->cap <= m->len) {
    debug("map is full!\n");
    return false;
  }

  debug("calling hash function to hash key\n");
  // hash the key and find the location
  uint32_t loc = m->hashFunc(key, len) % m->cap;

  // find an empty bucket
  while (m->map[loc] != NULL) {
    // check if key has been inserted before
    debug("checking if key has been inserted before\n");
    if ((len == 1 && m->map[loc]->name == key[0]) ||
        (m->map[loc]->name_long != NULL &&
         !strncmp(m->map[loc]->name_long, key, len))) {
      debug("flag has been inserted before\n");
      return false;
    }
    loc = (loc + 1) % m->cap;
  }

  m->map[loc] = v;
  m->len++;
  debug("flag inserted into map successfully\n");
  return true;
}

flag_t *map_find(map_t *m, const char *key, size_t len) {

  // panic if key is NULL
  assert(key != NULL);

  // return if map is empty
  if (m->len == 0) {
    return NULL;
  }

  uint32_t loc = m->hashFunc(key, len) % m->cap;
  while (m->map[loc] != NULL) {
    // check if key has been inserted before
    if ((len == 1 && m->map[loc]->name == key[0]) ||
        (m->map[loc]->name_long != NULL &&
         !strncmp(m->map[loc]->name_long, key, len))) {
      debug("%c key found\n", *key);
      return m->map[loc];
    }
    loc = (loc + 1) % m->cap;
  }
  debug("%c key not found\n", *key);
  return NULL;
}

flag_arena_t *flag_arena_new() {
  flag_arena_t *arena = (flag_arena_t *)malloc(
      sizeof(flag_arena_t) + (sizeof(flag_t) * CFLAG_FLAG_ARENA_INIT_CAP));
  if (arena) {
    arena->used = 0;
    arena->cap = CFLAG_FLAG_ARENA_INIT_CAP;
    arena->next = NULL;
  }
  return arena;
}

flag_t *flag_arena_alloc(flag_arena_t *arena) {
  while (arena->used >= arena->cap) {
    if (arena->next == NULL) {
      arena->next = flag_arena_new();
    }
    arena = arena->next;
  }
  return &arena->mem[arena->used++];
}

void flag_arena_delete(flag_arena_t *arena) {
  flag_arena_t *next = NULL, *curr = arena;
  while (curr) {
    next = curr->next;
    free(curr);
    curr = next;
  }
}

size_t flag_arena_len(flag_arena_t *arena) {
  flag_arena_t *curr = arena;
  size_t len = 0;
  while (curr) {
    len += curr->used;
    curr = curr->next;
  }
  return len;
}

flag_t *flag_arena_at(flag_arena_t *arena, size_t index) {
  while (index >= arena->used) {
    index -= arena->used;
    arena = arena->next;
    if (arena == NULL)
      return NULL;
  }
  return &arena->mem[index];
}

int parse_bool(flag_t *f, const char *arg) {
  // BUG: multiple calls, causes different results
  // TODO: arbitrary number of calls, must only result in reverse of default
  // value
  *(bool *)f->dest = !(*(bool *)f->dest);
  return PARSE_ARG_REMAINED;
}

int parse_int(flag_t *f, const char *arg) {
  // TODO: instead of "strlen(arg) == 0" use arg[0] == '\0'
  if (arg == NULL || strlen(arg) == 0) {
    return PARSE_ARG_NEEDED;
  }

  char *endptr;
  int n = strtol(arg, &endptr, 0);
  if (*endptr != '\0') {
    return PARSE_ARG_INVALID;
  }

  *(int *)f->dest = n;

  return PARSE_ARG_CONSUMED;
}

int parse_uint(flag_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return PARSE_ARG_NEEDED;
  }

  char *endptr;
  uint n = strtoul(arg, &endptr, 0);
  if (*endptr != '\0') {
    return PARSE_ARG_INVALID;
  }

  *(uint *)f->dest = n;

  return PARSE_ARG_CONSUMED;
}

int parse_int64(flag_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return PARSE_ARG_NEEDED;
  }

  char *endptr;
  int64_t n = strtoll(arg, &endptr, 0);
  if (*endptr != '\0') {
    return PARSE_ARG_INVALID;
  }

  *(int64_t *)f->dest = n;

  return PARSE_ARG_CONSUMED;
}

int parse_uint64(flag_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return PARSE_ARG_NEEDED;
  }

  char *endptr;
  uint64_t n = strtoull(arg, &endptr, 0);
  if (*endptr != '\0') {
    return PARSE_ARG_INVALID;
  }

  *(uint64_t *)f->dest = n;

  return PARSE_ARG_CONSUMED;
}

int parse_float(flag_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return PARSE_ARG_NEEDED;
  }

  char *endptr;
  float n = strtof(arg, &endptr);
  if (*endptr != '\0') {
    return PARSE_ARG_INVALID;
  }

  *(float *)f->dest = n;

  return PARSE_ARG_CONSUMED;
}

int parse_double(flag_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return PARSE_ARG_NEEDED;
  }

  char *endptr;
  double n = strtod(arg, &endptr);
  if (*endptr != '\0') {
    return PARSE_ARG_INVALID;
  }

  *(double *)f->dest = n;

  return PARSE_ARG_CONSUMED;
}

int parse_string(flag_t *f, const char *arg) {
  // TODO: *arg == '\0' might have better performance
  if (arg == NULL || strlen(arg) == 0) {
    return PARSE_ARG_NEEDED;
  }

  *(char **)f->dest = (char *)arg;
  return PARSE_ARG_CONSUMED;
}

int find_max_len(flag_arena_t *flags) {
  int max_len = 0;
  size_t arena_len = flag_arena_len(flags);
  debug("arena_len is %llu\n", arena_len);
  for (size_t i = 0; i < arena_len; ++i) {
    flag_t *f = flag_arena_at(flags, i);
    debug("flag is %p\n", f);
    int curr_len = STRLEN(f->name_long) + STRLEN(f->arg_name);
    max_len = (curr_len > max_len) ? curr_len : max_len;
  }
  return max_len;
}

// TODO: print_flags sucks, it needs to be suckless
void print_flags(flag_arena_t *flags) {
  int nspaces = find_max_len(flags);
  nspaces += 10;
  debug("maximum len is %d\n", nspaces);
  size_t arena_len = flag_arena_len(flags);
  for (size_t i = 0; i < arena_len; ++i) {
    int n = nspaces;

    char buff[1024] = {'\0'};
    flag_t *f = flag_arena_at(flags, i);
    // name, name_long=<arg_name> usage
    if (f->name) {
      printf("  -%c", f->name);
    } else {
      printf("    ");
    }

    if (f->name && f->name_long) {
      n--;
      putchar(',');
    }

    if (f->name_long) {
      snprintf(buff, sizeof(buff), " --%s", f->name_long);
      if (f->arg_name) {
        snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), "=<%s>",
                 f->arg_name);
      }
      printf("%-*s", n, buff);
    } else {
      if (f->arg_name) {
        snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), " <%s>",
                 f->arg_name);
      }
      printf("%-*s", n, buff);
    }
    printf("%s\n", f->usage ? f->usage : "");
  }
}

void print_err(int err_code, const char *prog_name, bool is_short,
               const char *opt, size_t opt_len, const char *arg) {

  // TODO: gnu seems to print different error message base on short or long
  // format is it really necessary in this library?
  //
  //
  // TODO: Add '--' before long options
  char buff[1024];
  const char *invalid_opt_err, *invalid_arg_err, *need_arg_err;
  if (is_short) {
    invalid_opt_err = "%s: invalid option -- '%s'\n";
    invalid_arg_err = "%s: invalid '%s' argument: '%s'\n";
    need_arg_err = "%s: option requires an argument -- '%s'\n";
    buff[0] = *opt;
    buff[1] = '\0';
  } else {
    invalid_opt_err = "%s: unrecognize option -- '%s'\n";
    invalid_arg_err = "%s: invalid %s argument: '%s'\n";
    need_arg_err = "%s: option '%s' requires an argument\n";
    memcpy(buff, opt, (1024 > opt_len ? opt_len : 1023));
    buff[(1024 > opt_len ? opt_len : 1023)] = '\0';
  }

  switch (err_code) {

  case PARSE_OPT_INVALID:
    fprintf(stderr, invalid_opt_err, prog_name, buff);

    break;
  case PARSE_ARG_INVALID:
    fprintf(stderr, invalid_arg_err, prog_name, buff, arg);
    break;

  case PARSE_ARG_NEEDED:
    fprintf(stderr, need_arg_err, prog_name, buff);

    break;
  }

  // TODO: print usage then exit
  exit(1);
}
