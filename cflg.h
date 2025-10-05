/*
 * cflg - Simple, STB-Style Command-Line Flag Parsing for C
 *
 * Version: v1.0, Repository: `https://github.com/ali-hasehmi/cflg`
 * Copyright: (c) 2025 Alireza G.Hashemi. Licensed under MIT (see LICENSE file).
 *
 * Overview:
 * -------------
 *   cflg is a single-header C library for command-line flag parsing, inspired by Go's
 *   flag package. It provides a modern, efficient API that is malloc-free (stack-only),
 *   portable across C99+ compilers, and designed for simplicity in integration,
 *   extensibility and use.
 *   The library emphasizes direct binding of flags to user variables, automatic help
 *   generation, and extensibility through custom parsers, making it ideal for CLI tools
 *   where reliability and minimal overhead are critical.
 *
 * Features:
 * -------------
 *   - Direct variable binding: Flags map to your bool, int, string, float, etc., variables.
 *   - Automatic --help: Generates usage messages from flag definitions.
 *   - Aggregated short options: Parses -vqc as separate -v -q -c.
 *   - Long option completion: Resolves --he to --help if unambiguous.
 *   - Custom parsers: Handles complex types like --memory=512m.
 *   - Positional rearrangement: Moves non-options first after argv[0].
 *   - Duplicate prevention: Tracks parsed flags with has_been_parsed.
 *   - Numeric parsing: Uses strtod and variants for robust integer/float handling.
 *
 * Usage:
 * -------------
 *   1. Copy cflg.h into your project directory.
 *
 *   2. In one C file (e.g., main.c), define:
 *
 *   ```c
 *      #define CFLG_IMPLEMENTATION
 *      #include "cflg.h"
 *   ```
 *
 *   3. Initialize a cflg_flgset_t and define flags:
 *
 *   ```c
 *      cflg_flgset_t fset = {0};
 *      cflg_flgset_string(&fset, &my_string, 's', "string", "<VAL>", "Description");
 *      cflg_flgset_bool(&fset, &my_bool, 'b', "bool", "Description");
 *   ```
 *
 *   4. Parse arguments with cflg_flgset_parse(&fset, argc, argv).
 *
 *   5. Access values via bound variables and positionals via fset.narg/fset.args.
 *   See README.md for quick-start examples or examples/ (wget.c, docker.c, ping.c)
 *   for advanced usage.
 *
 * Configuration:
 * -------------
 *
 *   - PROGRAM_NAME: Sets the program name in usage messages (default: argv[0]).
 *     Use this for custom binaries or when argv[0] is unreliable.
 *
 *   - CFLG_NO_SHORT_NAMES: Disables short aliases (e.g., use cflg_flgset_bool
 *     instead of flgset_bool) to avoid naming conflicts with other libraries.
 *     Ideal for large projects or when integrating with conflicting APIs.
 *
 * How To Extend
 * -------------
 * cflg is fully extensible for custom data types (e.g., durations like -t 5m or
 * --time=12s) via two methods: using cflg_flgset_func for one-off parsers or
 * creating a wrapper header for reusable macros.
 *
 *   Method 1: Using cflg_flgset_func
 *
 *     Define a custom parser function (type cflg_parser_t) that processes the
 *     argument in cflg_parser_context_t(see its defnintion) and stores the result in ctx->dest. Use
 *     cflg_flgset_func to bind the parser to a flag.
 *
 *     Example (duration parser for "5m", "2h30m"):
 *     [WARNING: This example is AI Generated, never use this code if you don't understand it]
 *
 *     ```c
 *        int parse_duration(cflg_parser_context_t *ctx) {
 *          if (!ctx->arg) return CFLG_ERR_ARG_NEEDED;
 *          long long duration = 0;
 *          char *p = ctx->arg, *end;
 *          while (*p) {
 *            long val = strtol(p, &end, 10);
 *            if (p == end) return CFLG_ERR_INVALID_ARG;
 *            switch (*end) {
 *              case 'h': duration += val * 3600; break;
 *              case 'm': duration += val * 60; break;
 *              case 's': duration += val; break;
 *              default: return CFLG_ERR_INVALID_ARG;
 *            }
 *            p = end + 1;
 *          }
 *          *(long long *)ctx->dest = duration;
 *          ctx->has_been_parsed = true;
 *          return CFLG_OK;
 *        }
 *     ```
 *
 *     Usage:
 *
 *     ```c
 *       long long timeout = 0;
 *       cflg_flgset_t fset = {0};
 *       cflg_flgset_func(&fset, &timeout, 't', "timeout", "<DURATION>", "Timeout (e.g., 5m)", parse_duration);
 *       cflg_flgset_parse(&fset, argc, argv);
 *     ```
 *
 *   Method 2: Creating a Wrapper Header File
 *
 *     Create a header (e.g., cflg-wrapper.h) to define reusable macros for custom
 *     types using cflg_new_flag. This maps new flag types (e.g., cflg_flgset_duration)
 *     to a default parser, simplifying usage across projects.
 *
 *     Example (cflg-wrapper.h):
 *     [WARNING: This example is AI Generated, never use this code if you don't understand it]
 *
 *     ```c
 *       #ifndef CFLG_WRAPPER_H
 *       #define CFLG_WRAPPER_H
 *       #include "cflg.h"
 *       #define cflg_flgset_duration(flgset, var, opt, opt_long, arg, desc) \
 *         cflg_new_flag(flgset, parse_duration, var, opt, opt_long, arg, desc)
 *       #endif
 *      // parse_duration implementation...
 *      // it is same as the above example...
 *      // ...
 *      // ...
 *     ```
 *
 *     Usage:
 *     ```c
 *       #include "cflg-wrapper.h"
 *       long long timeout = 0;
 *       cflg_flgset_t fset = {0};
 *       cflg_flgset_duration(&fset, &timeout, 't', "timeout", "<DURATION>", "Timeout (e.g., 5m)");
 *       cflg_flgset_parse(&fset, argc, argv);
 *     ```
 *
 *     This method is ideal for libraries or large projects needing consistent custom
 *     types across multiple files.
 *
 *
 * API Reference
 * -------------
 *
 *   Data Types
 *   ----------
 *     - cflg_flgset_t: Main flag set structure containing flags, positionals, and state.
 *       Fields: prog_name, narg, args, flags (linked list).
 *     - cflg_flg_t: Individual flag definition (name, name_long, parser, dest, usage).
 *     - cflg_parser_context_t: Context passed to custom parsers (opt, arg, dest, state).
 *     - cflg_usage_t: Function pointer for custom help handlers (void (*)(cflg_flgset_t*)).
 *
 *   Core Functions and Macros
 *   -------------------------
 *    Default Flag Definition Macros (bind flags to variables):
 *     - cflg_flgset_bool(fset, &var, 'b', "bool", "Usage"): Boolean flag.
 *     - cflg_flgset_int(fset, &var, 'i', "int", "<NUM>", "Usage"): Signed int.
 *     - cflg_flgset_int64(fset, &var, 'I', "int64", "<NUM>", "Usage"): 64-bit signed int.
 *     - cflg_flgset_uint(fset, &var, 'u', "uint", "<NUM>", "Usage"): Unsigned int.
 *     - cflg_flgset_uint64(fset, &var, 'U', "uint64", "<NUM>", "Usage"): 64-bit unsigned int.
 *     - cflg_flgset_float(fset, &var, 'f', "float", "<VAL>", "Usage"): Float.
 *     - cflg_flgset_double(fset, &var, 'd', "double", "<VAL>", "Usage"): Double.
 *     - cflg_flgset_string(fset, &var, 's', "string", "<STR>", "Usage"): String.
 *     - cflg_flgset_func(fset, &var, 'x', "custom", "<ARG>", "Usage", parser_func): Custom parser.
 *     - cflg_new_flag(fset, parser_func, &var, opt, opt_long, arg, Usage): Low-level flag creation.
 *
 *   Core Functions
 *   --------------
 *     - void cflg_flgset_parse(cflg_flgset_t *fset, int argc, char **argv): Parse arguments.
 *     - void cflg_print_flags(cflg_flg_t *flags): Print flags with alignment.
 *
 * Return Codes (from cflg_parser_t):
 * -------------
 *   - CFLG_OK: Option parsed, argument consumed.
 *   - CFLG_OK_NO_ARG: Option parsed, no argument needed.
 *   - CFLG_ERR_ARG_NEEDED: Requires argument, none provided.
 *   - CFLG_ERR_ARG_INVALID: Argument invalid (e.g., wrong format).
 *   - CFLG_ERR_ARG_FORCED: Argument consumption is mandatory, but you don't need it
 *     even if `CFLG_OK_NO_ARG` is returned, automatically generates error
 *
 * Dependencies: stdint.h ,stdbool.h, stdio.h, string.h, stdlib.h (for strtod parsing) (C99+).
 *
 * Notes:
 * -------------
 *   - Positional arguments are rearranged to follow argv[0] in fset.args,
 *     with count in fset.narg. Avoid re-parsing argv after cflg_flgset_parse.
 *   - Custom parsers extend via cflg_flgset_func; see examples/ for usage.
 *   - Aggregated short options (-vqc) are split into individual flags.
 *   - Long option completion resolves unambiguous prefixes (e.g., --he for --help).
 *   - Numeric parsing uses strtod and variants for safe, locale-aware int/float handling.
 *
 *
 *
 */

//
// ******                          ******
// ******                          ******
// ******   HEADER SECTION START   ******
// ******                          ******
// ******                          ******
//
#ifndef CFLG_H_INCLUDE
#define CFLG_H_INCLUDE

#include <stdbool.h> // bool
#include <stdint.h>  // uint, uint64_t, int32_t, int64_t

#ifndef CFLG_NO_SHORT_NAMES
#define parser_context_t cflg_parser_context_t
#define flg_t            cflg_flg_t
#define flgset_t         cflg_flgset_t
#define flgset_parse     cflg_flgset_parse
#define flgset_int       cflg_flgset_int
#define flgset_uint      cflg_flgset_uint
#define flgset_int64     cflg_flgset_int64
#define flgset_uint64    cflg_flgset_uint64
#define flgset_string    cflg_flgset_string
#define flgset_bool      cflg_flgset_bool
#define flgset_float     cflg_flgset_float
#define flgset_double    cflg_flgset_double
#define flgset_func      cflg_flgset_func
#define print_flags      cflg_print_flags
#define OK               CFLG_OK
#define OK_NO_ARG        CFLG_OK_NO_ARG
#define ERR_ARG_NEEDED   CFLG_ERR_ARG_NEEDED
#define ERR_ARG_INVALID  CFLG_ERR_ARG_INVALID
#define ERR_ARG_FORCED   CFLG_ERR_ARG_FORCED
#endif

// stores the state of each command-line option while parsing
typedef struct {
    const char *opt; // points the current command-line option,
                     // always check opt_len, it might not be null terminated.

    uint32_t opt_len; // stores the length of the current command-line option.
                      // always 1, if option is short

    bool is_opt_short; // stores whether the option is short(e.g. -v)
                       // or long(e.g. --verbose, --v, --ver).

    bool has_been_parsed; // stores whether the corresponding flag was used before

    bool is_arg_forced; // is option's argument forced (e.g. '--verbose=3' ).

    void *dest; // points to the user specified memory

    const char *arg; // points to the option's argument
                     // which is always null terminated, NULL if not provided.

} cflg_parser_context_t;

/* parser function type
   Function pointer type for parsing a command-line flag's argument.
   Takes a cflg_parser_context_t* containing flag details and argument,
   returns a status code (e.g., CFLG_OK, CFLG_ERR_ARG_NEEDED). */
typedef int (*cflg_parser_t)(cflg_parser_context_t *);

/* Return values for command-line flag parser functions */
#define CFLG_OK              0  /* Successfully parsed option and consumed its argument */
#define CFLG_OK_NO_ARG       1  /* Successfully parsed option; no argument was required or taken */
#define CFLG_ERR_ARG_NEEDED  -1 /* Parsing failed: option requires an argument, but none was provided */
#define CFLG_ERR_ARG_INVALID -2 /* Parsing failed: option's argument was provided but invalid (e.g., wrong format) */
#define CFLG_ERR_ARG_FORCED  -3 /* Parsing failed: option's argument was forced but not needed */

typedef struct cflg_flg cflg_flg_t;
struct cflg_flg {
    cflg_parser_t parser;    // parser binded with this flag
    void         *dest;      // memory region binded with this flag
    const char   *usage;     // usage message, use in printing
    const char   *arg_name;  // argument's name if need any
    const char   *name_long; // long option (e.g. '--verbose')
    char          name;      // short option (e.g. '-v')
    bool          has_seen;  // reports whether its parser has been called before
    cflg_flg_t   *next;      // points to the next flag in the linked-list
};

typedef struct cflg_flgset cflg_flgset_t;

/* Function pointer type for handling help requests when parsing '-h' or '--help'.
   Called with a cflg_flgset_t* containing flag definitions to display help output. */
typedef void (*cflg_usage_t)(cflg_flgset_t *);

struct cflg_flgset {
    bool         parsed;    // prevents parsing, if true
    int          narg;      // Number of non-flag arguments in args, including argv[0]
    char       **args;      // Pointer to argv used for parsing
    const char  *prog_name; // name of the program
    cflg_flg_t  *flgs;      // pointer to the head of the flags list
    cflg_usage_t usage;     // if not specified, falls back to default usage function
                            // (see cflg_print_help_)
};

#define CFLG_FALLBACK(s, def) ((s) ? (s) : (def))

// cflg_new_flag has been implemented using c99 compound literals
#define cflg_new_flag(flgset, parse_function, var, opt, opt_long, arg, desc)                                           \
    ((flgset)->flgs = &(cflg_flg_t) {.name      = (opt),                                                               \
                                     .name_long = (opt_long),                                                          \
                                     .parser    = (parse_function),                                                    \
                                     .dest      = (var),                                                               \
                                     .usage     = (desc),                                                              \
                                     .arg_name  = (arg),                                                               \
                                     .next      = (flgset)->flgs})

#define cflg_flgset_int(flgset, p, name, name_long, arg_name, usage)                                                   \
    cflg_new_flag((flgset), (cflg_parse_int), (int *) (p), (name), (name_long), CFLG_FALLBACK((arg_name), "int"),      \
                  (usage))

#define cflg_flgset_bool(flgset, p, name, name_long, usage)                                                            \
    cflg_new_flag((flgset), (cflg_parse_bool), (bool *) (p), (name), (name_long), NULL, (usage))

#define cflg_flgset_string(flgset, p, name, name_long, arg_name, usage)                                                \
    cflg_new_flag((flgset), (cflg_parse_string), (char **) (p), (name), (name_long),                                   \
                  CFLG_FALLBACK((arg_name), "string"), (usage))

#define cflg_flgset_float(flgset, p, name, name_long, arg_name, usage)                                                 \
    cflg_new_flag((flgset), (cflg_parse_float), (float *) (p), (name), (name_long),                                    \
                  CFLG_FALLBACK((arg_name), "float"), (usage));

#define cflg_flgset_uint(flgset, p, name, name_long, arg_name, usage)                                                  \
    cflg_new_flag((flgset), (cflg_parse_uint), (unsigned int *) (p), (name), (name_long),                              \
                  CFLG_FALLBACK((arg_name), "uint"), (usage));

#define cflg_flgset_int64(flgset, p, name, name_long, arg_name, usage)                                                 \
    cflg_new_flag((flgset), (cflg_parse_int64), (int64_t *) (p), (name), (name_long),                                  \
                  CFLG_FALLBACK((arg_name), "int64"), (usage))

#define cflg_flgset_uint64(flgset, p, name, name_long, arg_name, usage)                                                \
    cflg_new_flag((flgset), (cflg_parse_uint64), (uint64_t *) (p), (name), (name_long),                                \
                  CFLG_FALLBACK((arg_name), "uint64"), (usage))

#define cflg_flgset_double(flgset, p, name, name_long, arg_name, usage)                                                \
    cflg_new_flag((flgset), (cflg_parse_double), (double *) (p), (name), (name_long),                                  \
                  CFLG_FALLBACK((arg_name), "double"), (usage));

#define cflg_flgset_func(flgset, p, name, name_long, arg_name, usage, parser)                                          \
    cflg_new_flag((flgset), (parser), (p), (name), (name_long), (arg_name), (usage))

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

// expose the defualt print flags function
void cflg_print_flags(cflg_flg_t *flags);

// sorts flag list lexicographically and updates the head 
void cflg_sort_flags(cflg_flg_t **flg_head);

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
//

#include <stdio.h>  // fprintf
#include <stdlib.h> // strtod, strtof, ...
#include <string.h> // strlen, strcmp, memcmp

// invalid option error value
#define CFLG_ERR_OPT_INVALID -4
// a long option cannot be auto completed
#define CFLG_ERR_OPT_AMBIGUOUS -5

// no more argument to parse, returns from cflg_flgset_parse_one in case of finishing parsing
#define CFLG_PARSE_FINISH 2
// the last argv element that was parsed was a non-flag
#define CFLG_PARSE_NONFLG 3

#define CFLG_STRLEN(s)          ((s) ? (strlen(s)) : (0))
#define CFLG_ISEMPTY(s)         (((s) == NULL) || (*(s) == '\0'))
#define CFLG_STRNCMP(s1, s2, n) (((s1) && (s2)) ? (strncmp((s1), (s2), (n))) : (-1))

// checks wheter the provided string has the structure of a flag or not
#define CFLG_IS_NONFLG(s) (CFLG_ISEMPTY((s)) || (s)[0] != '-' || (s)[1] == '\0')

#define CFLG_ISHELP(f)         ((f) == 'h')
#define CFLG_ISHELP_LONG(f, l) ((l) == strlen("help") && !memcmp((f), "help", l))

#define CFLG_FOREACH(item, flgs) for (cflg_flg_t *item = (flgs); item != NULL; item = item->next)

void cflg_print_err(int err_code, cflg_flgset_t *fs, cflg_parser_context_t *ctx);

// default print usage function
// prints usage string and all flag names (short and long)
void cflg_print_help_(cflg_flgset_t *fset) {
    printf("Usage: %s [OPTION]... [COMMAND]...\n\n", fset->prog_name);
    cflg_sort_flags(&fset->flgs);
    cflg_print_flags(fset->flgs);
}

const char *cflg_find_base(const char *path) {
    for (int i = strlen(path) - 1; i >= 0; --i) {
        if (path[i] == '/' || path[i] == '\\') {
            return path + i + 1;
        }
    }
    return path;
}

int cflg_flg_find_long(cflg_flg_t *flgs, const char *opt, uint32_t opt_len, cflg_flg_t **res) {
    *res           = NULL;
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

void cflg_swap_args(char *argv[], int i, int j) {
    if (i != j) {
        char *tmp = argv[i];
        argv[i]   = argv[j];
        argv[j]   = tmp;
    }
}

int cflg_flgset_parse_one_long(cflg_flgset_t *fs, int argc, char *argv[], cflg_parser_context_t *ctx, int *curr_index) {

    // search for '=' in flag
    ctx->arg = strchr(ctx->opt, '=');
    // if there is '=' (e.g. --count=3)
    if (ctx->arg) {
        ctx->is_arg_forced = true;
        ctx->opt_len       = ctx->arg - ctx->opt;
        ctx->arg++;
    }
    // if there is NOT '=' (e.g. --count 3)
    else {
        ctx->is_arg_forced = false;
        ctx->opt_len       = CFLG_STRLEN(ctx->opt);
        ctx->arg           = argv[*curr_index + 1];
    }

    cflg_flg_t *f   = NULL;
    int         res = cflg_flg_find_long(fs->flgs, ctx->opt, ctx->opt_len, &f);
    if (res != CFLG_OK) {
        return res;
    }

    // if ctx->arg is an empty string (e.g. --count=)
    if (CFLG_ISEMPTY(ctx->arg)) {
        ctx->arg = NULL;
    }
    ctx->has_been_parsed = f->has_seen;
    ctx->dest            = f->dest;

    res = f->parser(ctx);

    if (res == CFLG_OK_NO_ARG) {
        if (ctx->is_arg_forced) {
            return CFLG_ERR_ARG_FORCED;
        }
        ctx->opt = NULL;
    } else {
        // if it's CFLG_OK, CFLG_ERR_ARG_NEEDED, CFLG_ERR_ARG_INVALID or CFLG_ERR_ARG_FORCED
        if (res == CFLG_OK) {
            ctx->opt = NULL;
        }
        if (!ctx->is_arg_forced) {
            ++(*curr_index);
        }
    }

    f->has_seen = true;

    return res;
}

int cflg_flgset_parse_one(cflg_flgset_t *fs, int argc, char *argv[], cflg_parser_context_t *ctx, int *curr_index) {

    if (fs->parsed) {
        return CFLG_PARSE_FINISH;
    }

    // Advance to the next argv element
    if (CFLG_ISEMPTY(ctx->opt)) {

        ++(*curr_index);
        if (*curr_index == argc) {
            fs->parsed = true;
            return CFLG_PARSE_FINISH;
        }

        const char *curr_arg = argv[*curr_index];
        if (CFLG_IS_NONFLG(curr_arg)) {
            return CFLG_PARSE_NONFLG;
        }

        // get rid of the '-'
        ctx->opt = curr_arg + 1;

        // if it startes with '--'
        if (ctx->opt[0] == '-') {
            // special argv element '--' means forced end of options
            // reports end of parsing to the caller
            if (ctx->opt[1] == '\0') {
                // prevents further parsing from future calls
                fs->parsed = true;
                return CFLG_PARSE_FINISH;
            }

            // get rid of the second '-'
            ctx->opt++;
            ctx->is_opt_short = false;
        }

        // if it's a short option
        else {
            ctx->is_opt_short = true;
            // short options' length are always 1
            ctx->opt_len = 1;
            // this flag is only revelant to long options (always false for non-long options)
            ctx->is_arg_forced = false;
        }
    }

    if (!ctx->is_opt_short) {
        return cflg_flgset_parse_one_long(fs, argc, argv, ctx, curr_index);
    }

    // ctx->is_opt_short is true, so process the the short option
    cflg_flg_t *f = NULL;
    CFLG_FOREACH(i, fs->flgs) {
        if (i->name == *(ctx->opt)) {
            f = i;
            break;
        }
    }
    if (f == NULL) {
        return CFLG_ERR_OPT_INVALID;
    }

    ctx->arg     = ctx->opt + 1;
    bool advance = false;
    if (CFLG_ISEMPTY(ctx->arg)) {
        advance  = true;
        ctx->arg = argv[*curr_index + 1];
    }

    ctx->has_been_parsed = f->has_seen;
    ctx->dest            = f->dest;

    int res = f->parser(ctx);
    if (res == CFLG_OK_NO_ARG) {
        ctx->opt++;
    } else {
        // it is either CFLG_OK, CFLG_ERR_ARG_NEEDED or CFLG_ERR_ARG_INVALID
        if (res == CFLG_OK) {
            ctx->opt = NULL;
        }
        if (advance) {
            ++(*curr_index);
        }
    }

    f->has_seen = true;

    return res;
}

int cflg_flgset_parse(cflg_flgset_t *fset, int argc, char *argv[]) {

    if (fset->parsed)
        return argc - fset->narg;

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

    fset->narg = 0;
    fset->args = argv;

    // if user didn't provide usage function, fall back to default
    if (fset->usage == NULL) {
        fset->usage = cflg_print_help_;
    }

    bool is_help = false;
    cflg_flgset_bool(fset, &is_help, 'h', "help", "print this help");

    int                   last_nonopt = 0;
    int                   curr_index  = 0; // argv[0] is always assumed to be the name of the executable
    cflg_parser_context_t ctx         = {0};

    for (;;) {
        int res = cflg_flgset_parse_one(fset, argc, argv, &ctx, &curr_index);
        if (res == CFLG_PARSE_FINISH) {
            break;
        }
        // if it was a non flag
        if (res == CFLG_PARSE_NONFLG) {
            cflg_swap_args(argv, curr_index, ++last_nonopt);
            continue;
        }
        // if there was an error
        if (res != CFLG_OK && res != CFLG_OK_NO_ARG) {
            cflg_print_err(res, fset, &ctx);
            exit(1);
        }
        // if help was requested
        if (is_help) {
            fset->usage(fset);
            is_help = false;
            exit(0);
        }
    }

    for (curr_index = curr_index + 1; curr_index < argc; ++curr_index) {
        cflg_swap_args(argv, curr_index, ++last_nonopt);
    }

    // remove help flag, because after returning from this function
    // it will be deallocated
    fset->flgs   = fset->flgs->next;

    fset->narg   = last_nonopt + 1;
    fset->parsed = true;

    // return the number of processed arguments
    return argc - fset->narg;
}

int cflg_parse_bool(cflg_parser_context_t *ctx) {
    if (!ctx->has_been_parsed) {
        *(bool *) ctx->dest = !(*(bool *) ctx->dest);
    }
    return CFLG_OK_NO_ARG;
}

int cflg_parse_int(cflg_parser_context_t *ctx) {
    if (ctx->arg == NULL) {
        return CFLG_ERR_ARG_NEEDED;
    }
    char *endptr;
    int   n = strtol(ctx->arg, &endptr, 0);
    if (*endptr != '\0') {
        return CFLG_ERR_ARG_INVALID;
    }

    *(int *) ctx->dest = n;

    return CFLG_OK;
}

int cflg_parse_uint(cflg_parser_context_t *ctx) {
    if (ctx->arg == NULL) {
        return CFLG_ERR_ARG_NEEDED;
    }
    char        *endptr;
    unsigned int n = strtoul(ctx->arg, &endptr, 0);
    if (*endptr != '\0') {
        return CFLG_ERR_ARG_INVALID;
    }

    *(unsigned int *) ctx->dest = n;

    return CFLG_OK;
}

int cflg_parse_int64(cflg_parser_context_t *ctx) {
    if (ctx->arg == NULL) {
        return CFLG_ERR_ARG_NEEDED;
    }

    char   *endptr;
    int64_t n = strtoll(ctx->arg, &endptr, 0);
    if (*endptr != '\0') {
        return CFLG_ERR_ARG_INVALID;
    }

    *(int64_t *) ctx->dest = n;

    return CFLG_OK;
}

int cflg_parse_uint64(cflg_parser_context_t *ctx) {
    if (ctx->arg == NULL) {
        return CFLG_ERR_ARG_NEEDED;
    }
    char    *endptr;
    uint64_t n = strtoull(ctx->arg, &endptr, 0);
    if (*endptr != '\0') {
        return CFLG_ERR_ARG_INVALID;
    }

    *(uint64_t *) ctx->dest = n;

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

    *(float *) ctx->dest = n;

    return CFLG_OK;
}

int cflg_parse_double(cflg_parser_context_t *ctx) {
    if (ctx->arg == NULL) {
        return CFLG_ERR_ARG_NEEDED;
    }
    char  *endptr;
    double n = strtod(ctx->arg, &endptr);
    if (*endptr != '\0') {
        return CFLG_ERR_ARG_INVALID;
    }

    *(double *) ctx->dest = n;

    return CFLG_OK;
}

int cflg_parse_string(cflg_parser_context_t *ctx) {
    if (ctx->arg == NULL) {
        return CFLG_ERR_ARG_NEEDED;
    }
    *(char **) ctx->dest = (char *) ctx->arg;
    return CFLG_OK;
}

// Helper function which converts c to lower case if it's uppercase
// otherwise returns c itself 
int cflg_tolower(int c) {
    if ( c >= 'A' && c <= 'Z') {
        return c + 'a' - 'A';
    }
    return c;
}

// compares two flags
// returns:
//  0 if equal => which is actually a bad situation
// <0 if a < b
// >0 if a > b
// on a correctly-defined flag list, no two flags should be equal
// calling this function with a->name = a->name_long = b->name = b->name_long = 0
// casues panic and segmentation fault so don't do it
int cflg_cmp_flgs(cflg_flg_t *a, cflg_flg_t *b) {
    
    // Determine the primary sort character for each flag.
    // Use the short option if it exists, otherwise the first letter of the long option.
    char key_a = CFLG_FALLBACK(a->name, a->name_long[0]);
    char key_b = CFLG_FALLBACK(b->name, b->name_long[0]);

    // 1. Primary comparison: case-insensitive
    int diff = cflg_tolower(key_a) - cflg_tolower(key_b);
    if (diff != 0) {
        return diff;
    }

    // 2. Tie-breaker 1: case-sensitive (for -a vs -A)
    diff = key_a - key_b;
    if (diff != 0) {
        return -diff; // first print -a then -A
    }

    // 3. Tie-breaker 2: full long option name (for -c vs --color)
    // A flag with no long option should come before one that has one.
    if (a->name_long == NULL && b->name_long != NULL) return -1;
    if (a->name_long != NULL && b->name_long == NULL) return 1;
    if (a->name_long != NULL && b->name_long != NULL) {
        return strcmp(a->name_long, b->name_long);
    }

    return 0; // Flags are identical(this is bad)
}

// sorts flag list lexicographically and updates the head 
void cflg_sort_flags(cflg_flg_t **flg_head) {
    // this function sorts flags using insertion sort
    
    // create a dummy flag node to avoid edge cases of changing the head
    cflg_flg_t *dummy = &(cflg_flg_t) {.next = *flg_head},
    // prev always points to the end of the sorted list
               *prev  = *flg_head,
    // curr always points to the start of the unsorted list
               *curr  = (*flg_head)->next;

    while (curr) {
        // if current is already in the correct location 
        if (cflg_cmp_flgs(curr,prev) > 0) {
            prev = curr;
            curr = curr->next;
            continue;
        }

        // otherwise find its correct location
        cflg_flg_t *tmp = dummy;
        while(cflg_cmp_flgs(curr, tmp->next) > 0) {
                tmp = tmp->next;
        }
        
        prev->next = curr->next;
        curr->next = tmp->next;
        tmp->next  = curr;
        curr = prev->next;
    }

    // update the head to the new head
    *flg_head = dummy->next;
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

void cflg_print_err(int err_code, cflg_flgset_t *fs, cflg_parser_context_t *ctx) {

    // TODO: gnu seems to print different error message
    // base on short or long format is it really
    // necessary in this library?
    //
    //
    fprintf(stderr, "%s: ", fs->prog_name);

    const char *invalid_opt_err, *invalid_arg_err, *need_arg_err, *forced_arg_err, *ambiguous_opt_err;
    if (ctx->is_opt_short) {
        invalid_opt_err = "invalid option -- '%.*s'";
        invalid_arg_err = "invalid '%.*s' argument: '%s'";
        need_arg_err    = "option requires an argument -- '%.*s'";
    } else {
        invalid_opt_err   = "unrecognize option '--%.*s'";
        invalid_arg_err   = "invalid --%.*s argument: '%s'";
        need_arg_err      = "option '--%.*s' requires an argument";
        forced_arg_err    = "option '--%.*s' doesn't allow an argument";
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
            if (ctx->opt_len < CFLG_STRLEN(item->name_long) && !CFLG_STRNCMP(item->name_long, ctx->opt, ctx->opt_len)) {
                fprintf(stderr, " '--%s'", item->name_long);
            }
        }

        break;
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "Try '%s --help' for more information.\n", fs->prog_name);
}

//
// ******                                  ******
// ******                                  ******
// ******   IMPLEMENTATION SECTION END     ******
// ******                                  ******
// ******                                  ******
//
#endif // IMPLEMENTATION
