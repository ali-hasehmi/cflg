#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Assume cflg.h is in the same directory or in the include path
#define CFLG_IMPLEMENTATION
#include "../cflg.h"

// A simple macro to convert boolean to a string for printing
#define btoa(a) ((a) ? "true" : "false")

// --- Custom Data Structures ---

// A structure to hold multiple header strings.
typedef struct {
  char **items;
  size_t count;
  size_t capacity;
} header_list_t;

// --- Custom Parsers ---

/**
 * @brief Custom parser for the --limit-rate flag.
 * Parses arguments like "100k", "2.5m" into bytes per second.
 */
int rate_limit_parser(flg_t *f, const char *arg) {
  if (arg == NULL || *arg == '\0') {
    return CFLG_PARSE_ARG_NEEDED;
  }

  char *endptr;
  double val = strtod(arg, &endptr);
  if (endptr == arg) {
    return CFLG_PARSE_ARG_INVALID; // Not a number
  }

  double multiplier = 1.0;
  if (*endptr != '\0') {
    switch (tolower(*endptr)) {
    case 'k':
      multiplier = 1024;
      break;
    case 'm':
      multiplier = 1024 * 1024;
      break;
    case 'g':
      multiplier = 1024 * 1024 * 1024;
      break;
    default:
      return CFLG_PARSE_ARG_INVALID; // Invalid suffix
    }
  }

  *(long long *)(f->dest) = (long long)(val * multiplier);
  return CFLG_PARSE_ARG_CONSUMED;
}

/**
 * @brief Custom parser for the --header flag.
 * Allows the flag to be specified multiple times, collecting all arguments.
 */
int header_parser(flg_t *f, const char *arg) {
  if (arg == NULL || *arg == '\0') {
    return CFLG_PARSE_ARG_NEEDED;
  }

  header_list_t *list = (header_list_t *)f->dest;

  // Grow the list if necessary
  if (list->count >= list->capacity) {
    size_t new_capacity = (list->capacity == 0) ? 8 : list->capacity * 2;
    char **new_items = realloc(list->items, new_capacity * sizeof(char *));
    if (!new_items) {
      // In a real app, handle memory allocation failure gracefully
      perror("realloc failed");
      return CFLG_PARSE_ARG_INVALID;
    }
    list->items = new_items;
    list->capacity = new_capacity;
  }

  // Add the new header (strdup makes a copy)
  list->items[list->count] = strdup(arg);
  if (!list->items[list->count]) {
    perror("strdup failed");
    return CFLG_PARSE_ARG_INVALID;
  }
  list->count++;

  return CFLG_PARSE_ARG_CONSUMED;
}

int main(int argc, char *argv[]) {
  // --- Variable Definitions ---
  // We need a variable to store the value of each flag.

  // Startup options
  bool show_version = false;
  char *output_file = NULL;
  char *log_file = NULL;
  bool append_output = false;
  bool quiet_mode = false;
  bool verbose_mode = false;

  // Download options
  int tries = 20;
  char *bind_address = NULL;
  int connect_timeout = 0; // 0 means default
  bool continue_download = false;
  bool ignore_length = false;
  long long limit_rate = 0; // New variable for rate limiting

  // HTTP options
  char *http_user = NULL;
  char *http_password = NULL;
  char *user_agent = NULL;
  bool no_cookies = false;
  header_list_t headers = {0}; // New structure for multiple headers

  // --- Flag Set Initialization ---
  flgset_t fset = {0}; // IMPORTANT: always initialize it with zero

  // --- Flag Definitions ---
  // Use the flgset_* functions to register each flag.

  // Startup and Logging
  flgset_bool(&fset, &show_version, 'V', "version",
              "display the version of wget and exit.");
  flgset_string(&fset, &output_file, 'O', "output-file", "<FILE>",
                "log messages to FILE.");
  flgset_string(&fset, &log_file, 'o', "output-log", "<FILE>",
                "log messages to FILE.");
  flgset_bool(&fset, &append_output, 'a', "append-output",
              "append messages to FILE.");
  flgset_bool(&fset, &quiet_mode, 'q', "quiet", "quiet (no output).");
  flgset_bool(&fset, &verbose_mode, 'v', "verbose",
              "be verbose (this is the default).");

  // Download
  flgset_int(&fset, &tries, 't', "tries", "<NUMBER>",
             "set number of retries to NUMBER (0 unlimits).");
  flgset_string(&fset, &bind_address, 0, "bind-address", "<ADDRESS>",
                "bind to ADDRESS (hostname or IP) on local host.");
  flgset_int(&fset, &connect_timeout, 'T', "timeout", "<SECONDS>",
             "set the read timeout to SECONDS.");
  flgset_bool(&fset, &continue_download, 'c', "continue",
              "resume getting a partially-downloaded file.");
  flgset_bool(&fset, &ignore_length, 0, "ignore-length",
              "ignore 'Content-Length' header field.");
  flgset_func(&fset, &limit_rate, 0, "limit-rate", "<RATE>",
              "limit download speed to RATE (e.g., 100k, 2.5m).",
              rate_limit_parser);

  // HTTP
  flgset_string(&fset, &http_user, 0, "http-user", "<USER>",
                "set http user to USER.");
  flgset_string(&fset, &http_password, 0, "http-password", "<PASS>",
                "set http password to PASS.");
  flgset_string(&fset, &user_agent, 'U', "user-agent", "<AGENT>",
                "identify as AGENT instead of Wget/VERSION.");
  flgset_bool(&fset, &no_cookies, 0, "no-cookies", "don't use cookies.");
  flgset_func(&fset, &headers, 0, "header", "<STRING>",
              "insert STRING among the headers sent.", header_parser);

  // --- Parsing ---
  struct timespec start, end;
  timespec_get(&start, TIME_UTC);
  flgset_parse(&fset, argc, argv);
  timespec_get(&end, TIME_UTC);

  // --- Post-Parsing Logic ---
  printf("--- Wget Flag Emulation Results ---\n\n");

  printf("Startup Options:\n");
  printf("  Verbose: %s\n", btoa(verbose_mode));
  printf("  Quiet: %s\n", btoa(quiet_mode));
  printf("  Output File: %s\n", output_file ? output_file : "(none)");
  printf("  Log File: %s\n", log_file ? log_file : "(none)");
  printf("  Append Output: %s\n", btoa(append_output));
  printf("\n");

  printf("Download Options:\n");
  printf("  Retries: %d\n", tries);
  printf("  Continue: %s\n", btoa(continue_download));
  printf("  Bind Address: %s\n", bind_address ? bind_address : "(default)");
  printf("  Timeout: %d seconds\n", connect_timeout);
  printf("  Ignore Content-Length: %s\n", btoa(ignore_length));
  printf("  Rate Limit: %lld bytes/sec\n", limit_rate);
  printf("\n");

  printf("HTTP Options:\n");
  printf("  User-Agent: %s\n", user_agent ? user_agent : "(default)");
  printf("  HTTP User: %s\n", http_user ? http_user : "(none)");
  printf("  HTTP Password: %s\n",
         http_password ? http_password : "(not shown)");
  printf("  No Cookies: %s\n", btoa(no_cookies));
  printf("  Custom Headers:\n");
  if (headers.count > 0) {
    for (size_t i = 0; i < headers.count; i++) {
      printf("    - \"%s\"\n", headers.items[i]);
    }
  } else {
    printf("    (none)\n");
  }
  printf("\n");

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Flags parsed in %.9f seconds\n", elapsed);

  // --- Cleanup ---
  // Free the memory allocated by our custom header parser.
  for (size_t i = 0; i < headers.count; i++) {
    free(headers.items[i]);
  }
  free(headers.items);

  return 0;
}
