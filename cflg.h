// ******                          ******
// ******                          ******
// ******   HEADER SECTION START   ******
// ******                          ******
// ******                          ******
#ifndef CFLG_H_INCLUDE
#define CFLG_H_INCLUDE

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CFLG_FLAG_ARENA_BLOCK_SIZE
#define CFLG_FLAG_ARENA_BLOCK_SIZE 10
#endif

#ifndef CFLG_NO_SHORT_NAMES
#define flgset_t cflg_flgset_t
#define flgset_create cflg_flgset_create
#define flgset_destroy cflg_flgset_destroy
#define flgset_parse cflg_flgset_parse
#define flgset_int cflg_flgset_int
#define flgset_uint cflg_flgset_uint
#define flgset_int64 cflg_flgset_int64
#define flgset_uint64 cflg_flgset_uint64
#define flgset_string cflg_flgset_string
#define flgset_bool cflg_flgset_bool
#define flgset_float cflg_flgset_float
#define flgset_double cflg_flgset_double
#endif

typedef struct cflg_flg {
  int (*parser)(struct cflg_flg *, const char *);
  void *dest;
  const char *usage;
  const char *arg_name;
  char name;
  const char *name_long;
} cflg_flg_t;

typedef struct {
  uint32_t (*hashFunc)(const char *, size_t);
  size_t len;
  size_t cap;
  cflg_flg_t **map;
} cflg_map_t;

typedef struct cflg_flg_arena {
  struct cflg_flg_arena *next;
  size_t used;
  size_t cap;
  cflg_flg_t mem[];
} cflg_flg_arena_t;

typedef struct {
  bool parsed;
  int narg;    // number of arguments remaining after flags have been processed.
  char **args; // non-flag arguments after flags have beeen processed.
  const char *prog_name; // name of the program
  cflg_flg_arena_t *arena;
} cflg_flgset_t;

void cflg_flgset_create(cflg_flgset_t *flgset);
void cflg_flgset_destroy(cflg_flgset_t *flgset);
// int  cflg_parse(int argc, char *argv[]);
void cflg_flgset_int(cflg_flgset_t *flgset, int *p, char name,
                     const char *name_long, const char *arg_name,
                     const char *usage);
void cflg_flgset_bool(cflg_flgset_t *flgset, bool *p, char name,
                      const char *name_long, const char *usage);
void cflg_flgset_string(cflg_flgset_t *flgset, char **p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage);
void cflg_flgset_float(cflg_flgset_t *flgset, float *p, char name,
                       const char *name_long, const char *arg_name,
                       const char *usage);
void cflg_flgset_uint(cflg_flgset_t *flgset, unsigned *p, char name,
                      const char *name_long, const char *arg_name,
                      const char *usage);
void cflg_flgset_int64(cflg_flgset_t *flgset, int64_t *p, char name,
                       const char *name_long, const char *arg_name,
                       const char *usage);
void cflg_flgset_uint64(cflg_flgset_t *flgset, uint64_t *p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage);

void cflg_flgset_double(cflg_flgset_t *flgset, double *p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage);

int cflg_flgset_parse(cflg_flgset_t *flgset, int argc, char *argv[]);

// ******                        ******
// ******                        ******
// ******   HEADER SECTION END   ******
// ******                        ******
// ******                        ******
//
#endif // CFLG_H_INCLUDE

#ifdef CFLG_IMPLEMENTATION
//  ******                                  ******
//  ******                                  ******
//  ******   IMPLEMENTATION SECTION START   ******
//  ******                                  ******
//  ******                                  ******

void cflg_map_create_from_arena(cflg_map_t *m, cflg_flg_arena_t *a);
void cflg_map_destroy(cflg_map_t *m);
bool cflg_map_insert(cflg_map_t *m, const char *k, size_t len, cflg_flg_t *v);
cflg_flg_t *cflg_map_find(cflg_map_t *m, const char *k, size_t len);

cflg_flg_arena_t *cflg_flg_arena_new();
void cflg_flg_arena_delete(cflg_flg_arena_t *arena);
cflg_flg_t *cflg_flg_arena_alloc(cflg_flg_arena_t *arena);
size_t cflg_flg_arena_len(cflg_flg_arena_t *arena);
cflg_flg_t *cflg_flg_arena_at(cflg_flg_arena_t *arena, size_t index);

#define CFLG_PARSE_ARG_REMAINED 0
#define CFLG_PARSE_ARG_CONSUMED 1
#define CFLG_PARSE_ARG_NEEDED -1
#define CFLG_PARSE_ARG_INVALID -2
#define CFLG_PARSE_OPT_INVALID -3

#define CFLG_STRLEN(s) ((s) ? (strlen(s)) : (0))
#define CFLG_FALLBACK(s, def) ((s) ? (s) : (def))

#define CFLG_ISHELP(f, l)                                                      \
  (l == 1 && f[0] == 'h') || (l == strlen("help") && !memcmp(f, "help", l))

int cflg_parse_bool(cflg_flg_t *f, const char *arg);
int cflg_parse_int(cflg_flg_t *f, const char *arg);
int cflg_parse_uint(cflg_flg_t *f, const char *arg);
int cflg_parse_int64(cflg_flg_t *f, const char *arg);
int cflg_parse_uint64(cflg_flg_t *f, const char *arg);
int cflg_parse_float(cflg_flg_t *f, const char *arg);
int cflg_parse_double(cflg_flg_t *f, const char *arg);
int cflg_parse_string(cflg_flg_t *f, const char *arg);

void cflg_print_flags(cflg_flg_arena_t *flags);
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

void cflg_flgset_create(cflg_flgset_t *flgset) {
  debug("start creating flagset\n");
  memset(flgset, 0, sizeof(cflg_flgset_t));
  flgset->arena = cflg_flg_arena_new();
  debug("end creating flagset\n");
}

void cflg_flgset_destroy(cflg_flgset_t *flgset) {
  if (flgset->args != NULL)
    free(flgset->args);
  cflg_flg_arena_delete(flgset->arena);
}

cflg_flg_t *cflg_new_flag(cflg_flg_arena_t *arena,
                          int (*parser)(cflg_flg_t *, const char *), void *dest,
                          char name, const char *name_long,
                          const char *arg_name, const char *usage) {
  if (name != 0 && !isalnum(name))
    return NULL;
  cflg_flg_t *f = cflg_flg_arena_alloc(arena);
  if (f) {
    memset(f, 0, sizeof(cflg_flg_t));
    f->parser = parser;
    f->name_long = name_long;
    f->name = name;
    f->usage = usage;
    f->dest = dest;
    f->arg_name = arg_name;
  }
  return f;
}

void cflg_flgset_int(cflg_flgset_t *flgset, int *p, char name,
                     const char *name_long, const char *arg_name,
                     const char *usage) {
  debug("start adding integer to flagset\n");
  cflg_flg_t *f =
      cflg_new_flag(flgset->arena, cflg_parse_int, p, name, name_long,
                    CFLG_FALLBACK(arg_name, "int"), usage);
}

void cflg_flgset_uint(cflg_flgset_t *flgset, unsigned *p, char name,
                      const char *name_long, const char *arg_name,
                      const char *usage) {
  debug("start adding unsigned integer to flagset\n");
  cflg_flg_t *f =
      cflg_new_flag(flgset->arena, cflg_parse_uint, p, name, name_long,
                    CFLG_FALLBACK(arg_name, "uint"), usage);
}

void cflg_flgset_int64(cflg_flgset_t *flgset, int64_t *p, char name,
                       const char *name_long, const char *arg_name,
                       const char *usage) {
  debug("start adding int64 to flagset\n");
  cflg_flg_t *f =
      cflg_new_flag(flgset->arena, cflg_parse_int64, p, name, name_long,
                    CFLG_FALLBACK(arg_name, "int64"), usage);
}

void cflg_flgset_uint64(cflg_flgset_t *flgset, uint64_t *p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage) {
  debug("start adding uint64 to flagset\n");
  cflg_flg_t *f =
      cflg_new_flag(flgset->arena, cflg_parse_uint64, p, name, name_long,
                    CFLG_FALLBACK(arg_name, "uint64"), usage);
}
void cflg_flgset_bool(cflg_flgset_t *flgset, bool *p, char name,
                      const char *name_long, const char *usage) {
  debug("start adding bool to flagset\n");
  cflg_flg_t *f = cflg_new_flag(flgset->arena, cflg_parse_bool, p, name,
                                name_long, NULL, usage);
}

void cflg_flgset_float(cflg_flgset_t *flgset, float *p, char name,
                       const char *name_long, const char *arg_name,
                       const char *usage) {
  debug("start adding bool to flagset\n");
  cflg_flg_t *f = cflg_new_flag(flgset->arena, cflg_parse_float, p, name, name_long,
                                CFLG_FALLBACK(arg_name, "float"), usage);
}

void cflg_flgset_double(cflg_flgset_t *flgset, double *p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage) {
  debug("start adding bool to flagset\n");
  cflg_flg_t *f = cflg_new_flag(flgset->arena, cflg_parse_double, p, name, name_long,
                                CFLG_FALLBACK(arg_name, "double"), usage);
}

void cflg_flgset_string(cflg_flgset_t *flgset, char **p, char name,
                        const char *name_long, const char *arg_name,
                        const char *usage) {
  debug("start adding bool to flagset\n");
  cflg_flg_t *f = cflg_new_flag(flgset->arena, cflg_parse_string, p, name, name_long,
                                CFLG_FALLBACK(arg_name, "string"), usage);
}

int cflg_flgset_parse(cflg_flgset_t *fset, int argc, char *argv[]) {

  if (fset->parsed)
    return 0;

  cflg_map_t flags;
  cflg_map_create_from_arena(&flags, fset->arena);

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
    cflg_map_destroy(&flags);
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
      if (CFLG_ISHELP(flag, flag_len)) {
        debug("--help detected printing flags and exiting\n");
        cflg_map_destroy(&flags);
        cflg_print_flags(fset->arena);
        exit(0);
      }

      debug("looking up the hashtable for flag\n");
      cflg_flg_t *f = cflg_map_find(&flags, flag, flag_len);
      if (f == NULL || f->name_long == NULL ||
          strncmp(f->name_long, flag, flag_len)) {
        cflg_map_destroy(&flags);
        cflg_print_err(CFLG_PARSE_OPT_INVALID, fset->prog_name, false, flag,
                       flag_len, arg);
      }
      int res = f->parser(f, arg);
      switch (res) {
      case CFLG_PARSE_ARG_CONSUMED:
        if (skip_next) {
          i++;
        }
        break;
      case CFLG_PARSE_ARG_REMAINED:
        break;
      default:
        cflg_map_destroy(&flags);
        cflg_print_err(res, fset->prog_name, false, flag, flag_len, arg);
      }
    }
    // if it's a short flag
    else {
      // iterate over all short flags
      for (int j = 1; j < len; ++j) {
        char *flag = argv[i] + j;
        if (CFLG_ISHELP(flag, 1)) {
          cflg_map_destroy(&flags);
          cflg_print_flags(fset->arena);
          exit(0);
        }
        cflg_flg_t *f = cflg_map_find(&flags, flag, 1);
        if (f == NULL || f->name == 0 || f->name != flag[0]) {
          cflg_map_destroy(&flags);
          cflg_print_err(CFLG_PARSE_OPT_INVALID, fset->prog_name, true, flag, 1,
                         NULL);
        }
        bool skip_next = false;
        char *arg = argv[i] + j + 1;
        if (j + 1 == len) { // TODO: OR "*arg == '\0'" which one is better?
          arg = argv[i + 1];
          skip_next = true;
        }
        int res = f->parser(f, arg);
        bool break_loop = false;
        switch (res) {
        case CFLG_PARSE_ARG_CONSUMED:
          break_loop = true;
          if (skip_next) {
            i++;
          }
          break;
        case CFLG_PARSE_ARG_REMAINED:
          break;
        default:
          cflg_map_destroy(&flags);
          cflg_print_err(res, fset->prog_name, true, flag, 1, arg);
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

  cflg_map_destroy(&flags);
  fset->parsed = true;
  return 0;
}

void parseflg(cflg_flgset_t *flgset, char *first, char *second) {
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

void cflg_map_create_from_arena(cflg_map_t *m, cflg_flg_arena_t *a) {
  size_t arena_len = cflg_flg_arena_len(a);
  m->map = (cflg_flg_t **)malloc(2 * arena_len * sizeof(cflg_flg_t *));
  // TODO: implement proper error handling
  assert(m->map);
  memset(m->map, 0, 2 * arena_len * sizeof(cflg_flg_t *));
  m->len = 0;
  m->cap = 2 * arena_len;
  m->hashFunc = djb2_hash;

  for (int i = 0; i < arena_len; ++i) {
    cflg_flg_t *f = cflg_flg_arena_at(a, i);
    if (f->name)
      cflg_map_insert(m, &f->name, 1, f);
    if (f->name_long)
      cflg_map_insert(m, f->name_long, strlen(f->name_long), f);
  }
}

void cflg_map_destroy(cflg_map_t *m) { free(m->map); }

bool cflg_map_insert(cflg_map_t *m, const char *key, size_t len,
                     cflg_flg_t *v) {
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

cflg_flg_t *cflg_map_find(cflg_map_t *m, const char *key, size_t len) {

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

cflg_flg_arena_t *cflg_flg_arena_new() {
  cflg_flg_arena_t *arena = (cflg_flg_arena_t *)malloc(
      sizeof(cflg_flg_arena_t) +
      (sizeof(cflg_flg_t) * CFLG_FLAG_ARENA_BLOCK_SIZE));
  if (arena) {
    arena->used = 0;
    arena->cap = CFLG_FLAG_ARENA_BLOCK_SIZE;
    arena->next = NULL;
  }
  return arena;
}

cflg_flg_t *cflg_flg_arena_alloc(cflg_flg_arena_t *arena) {
  while (arena->used >= arena->cap) {
    if (arena->next == NULL) {
      arena->next = cflg_flg_arena_new();
    }
    arena = arena->next;
  }
  return &arena->mem[arena->used++];
}

void cflg_flg_arena_delete(cflg_flg_arena_t *arena) {
  cflg_flg_arena_t *next = NULL, *curr = arena;
  while (curr) {
    next = curr->next;
    free(curr);
    curr = next;
  }
}

size_t cflg_flg_arena_len(cflg_flg_arena_t *arena) {
  cflg_flg_arena_t *curr = arena;
  size_t len = 0;
  while (curr) {
    len += curr->used;
    curr = curr->next;
  }
  return len;
}

cflg_flg_t *cflg_flg_arena_at(cflg_flg_arena_t *arena, size_t index) {
  while (index >= arena->used) {
    index -= arena->used;
    arena = arena->next;
    if (arena == NULL)
      return NULL;
  }
  return &arena->mem[index];
}

int cflg_parse_bool(cflg_flg_t *f, const char *arg) {
  // BUG: multiple calls, causes different results
  // TODO: arbitrary number of calls, must only result in reverse of default
  // value
  *(bool *)f->dest = !(*(bool *)f->dest);
  return CFLG_PARSE_ARG_REMAINED;
}

int cflg_parse_int(cflg_flg_t *f, const char *arg) {
  // TODO: instead of "strlen(arg) == 0" use arg[0] == '\0'
  if (arg == NULL || strlen(arg) == 0) {
    return CFLG_PARSE_ARG_NEEDED;
  }

  char *endptr;
  int n = strtol(arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(int *)f->dest = n;

  return CFLG_PARSE_ARG_CONSUMED;
}

int cflg_parse_uint(cflg_flg_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return CFLG_PARSE_ARG_NEEDED;
  }

  char *endptr;
  uint n = strtoul(arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(uint *)f->dest = n;

  return CFLG_PARSE_ARG_CONSUMED;
}

int cflg_parse_int64(cflg_flg_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return CFLG_PARSE_ARG_NEEDED;
  }

  char *endptr;
  int64_t n = strtoll(arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(int64_t *)f->dest = n;

  return CFLG_PARSE_ARG_CONSUMED;
}

int cflg_parse_uint64(cflg_flg_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return CFLG_PARSE_ARG_NEEDED;
  }

  char *endptr;
  uint64_t n = strtoull(arg, &endptr, 0);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(uint64_t *)f->dest = n;

  return CFLG_PARSE_ARG_CONSUMED;
}

int cflg_parse_float(cflg_flg_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return CFLG_PARSE_ARG_NEEDED;
  }

  char *endptr;
  float n = strtof(arg, &endptr);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(float *)f->dest = n;

  return CFLG_PARSE_ARG_CONSUMED;
}

int cflg_parse_double(cflg_flg_t *f, const char *arg) {
  if (arg == NULL || strlen(arg) == 0) {
    return CFLG_PARSE_ARG_NEEDED;
  }

  char *endptr;
  double n = strtod(arg, &endptr);
  if (*endptr != '\0') {
    return CFLG_PARSE_ARG_INVALID;
  }

  *(double *)f->dest = n;

  return CFLG_PARSE_ARG_CONSUMED;
}

int cflg_parse_string(cflg_flg_t *f, const char *arg) {
  // TODO: *arg == '\0' might have better performance
  if (arg == NULL || strlen(arg) == 0) {
    return CFLG_PARSE_ARG_NEEDED;
  }

  *(char **)f->dest = (char *)arg;
  return CFLG_PARSE_ARG_CONSUMED;
}

int cflg_find_max_len(cflg_flg_arena_t *flags) {
  int max_len = 0;
  size_t arena_len = cflg_flg_arena_len(flags);
  debug("arena_len is %llu\n", arena_len);
  for (size_t i = 0; i < arena_len; ++i) {
    cflg_flg_t *f = cflg_flg_arena_at(flags, i);
    debug("flag is %p\n", f);
    int curr_len = CFLG_STRLEN(f->name_long) + CFLG_STRLEN(f->arg_name);
    max_len = (curr_len > max_len) ? curr_len : max_len;
  }
  return max_len;
}

// TODO: cflg_print_flags sucks, it needs to be suckless
void cflg_print_flags(cflg_flg_arena_t *flags) {
  int nspaces = cflg_find_max_len(flags);
  nspaces += 10;
  debug("maximum len is %d\n", nspaces);
  size_t arena_len = cflg_flg_arena_len(flags);
  for (size_t i = 0; i < arena_len; ++i) {
    int n = nspaces;

    char buff[1024] = {'\0'};
    cflg_flg_t *f = cflg_flg_arena_at(flags, i);
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

void cflg_print_err(int err_code, const char *prog_name, bool is_short,
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

  case CFLG_PARSE_OPT_INVALID:
    fprintf(stderr, invalid_opt_err, prog_name, buff);

    break;
  case CFLG_PARSE_ARG_INVALID:
    fprintf(stderr, invalid_arg_err, prog_name, buff, arg);
    break;

  case CFLG_PARSE_ARG_NEEDED:
    fprintf(stderr, need_arg_err, prog_name, buff);

    break;
  }

  // TODO: print usage then exit
  exit(1);
}

// ******                                  ******
// ******                                  ******
// ******   IMPLEMENTATION SECTION END     ******
// ******                                  ******
// ******                                  ******
#endif // IMPLEMENTATION
