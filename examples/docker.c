#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CFLG_IMPLEMENTATION
#include "../cflg.h"

#define btoa(a) ((a) ? "true" : "false")

// --- Custom Data Structures ---

// Generic list for flags that can be specified multiple times
typedef struct {
  char **items;
  size_t count;
  size_t capacity;
} string_list_t;

// Structure for user (UID:GID)
typedef struct {
  uint16_t uid;
  uint16_t gid;
} user_t;

// --- Custom Parsers ---

/**
 * @brief Generic parser to add an argument to a dynamic list of strings.
 * Used for --env, --volume, --cap-add, --cap-drop, and --publish.
 */
int list_parser(parser_context_t* ctx) {

  string_list_t *list = (string_list_t *)ctx->dest;

  // Grow the list if necessary
  if (list->count >= list->capacity) {
    size_t new_capacity = (list->capacity == 0) ? 8 : list->capacity * 2;
    char **new_items = realloc(list->items, new_capacity * sizeof(char *));
    if (!new_items) {
      perror("realloc failed");
      return CFLG_PARSE_ARG_INVALID;
    }
    list->items = new_items;
    list->capacity = new_capacity;
  }

  // Add the new item (strdup makes a copy)
  list->items[list->count] = strdup(ctx->arg);
  if (!list->items[list->count]) {
    perror("strdup failed");
    return CFLG_PARSE_ARG_INVALID;
  }
  list->count++;

  return CFLG_PARSE_OK;
}

/**
 * @brief Custom parser for memory sizes (e.g., "1g", "512m").
 */
int mem_parser(parser_context_t *ctx) {

  size_t len = strlen(ctx->arg);
  if (len < 1)
    return CFLG_PARSE_ARG_INVALID;

  long long multi = 1;
  char suffix = ctx->arg[len - 1];
  int num_len = len;

  switch (suffix) {
  case 'k':
  case 'K':
    multi = 1024;
    num_len--;
    break;
  case 'm':
  case 'M':
    multi = 1024 * 1024;
    num_len--;
    break;
  case 'g':
  case 'G':
    multi = 1024 * 1024 * 1024;
    num_len--;
    break;
  }

  char num_str[256];
  strncpy(num_str, ctx->arg, num_len);
  num_str[num_len] = '\0';

  char *endptr;
  long long val = strtoll(num_str, &endptr, 10);
  if (*endptr != '\0')
    return CFLG_PARSE_ARG_INVALID;

  *(long long *)(ctx->dest) = val * multi;
  return CFLG_PARSE_OK;
}

/**
 * @brief Custom parser for user and group IDs (e.g., "1000:1000").
 */
int user_parser(parser_context_t* ctx) {

  // Make a mutable copy to safely use strtok or modify the string
  char arg_copy[256];
  strncpy(arg_copy, ctx->arg, sizeof(arg_copy) - 1);
  arg_copy[sizeof(arg_copy) - 1] = '\0';

  char *uid_str = arg_copy;
  char *gid_str = strchr(uid_str, ':');

  if (gid_str == NULL)
    return CFLG_PARSE_ARG_INVALID;
  *gid_str = '\0'; // Split the string
  gid_str++;

  char *endptr;
  long uid = strtol(uid_str, &endptr, 10);
  if (*endptr != '\0' || uid < 0 || uid > UINT16_MAX)
    return CFLG_PARSE_ARG_INVALID;

  long gid = strtol(gid_str, &endptr, 10);
  if (*endptr != '\0' || gid < 0 || gid > UINT16_MAX)
    return CFLG_PARSE_ARG_INVALID;

  ((user_t *)ctx->dest)->uid = (uint16_t)uid;
  ((user_t *)ctx->dest)->gid = (uint16_t)gid;
  return CFLG_PARSE_OK;
}

/**
 * @brief Helper function to print a list of strings and free its memory.
 */
void print_and_free_list(const char *title, string_list_t *list,
                         int label_width) {
  printf("  %-*s ", label_width, title);
  if (list->count == 0) {
    printf("(none)\n");
  } else {
    printf("\n");
    for (size_t i = 0; i < list->count; ++i) {
      printf("%*s- %s\n", label_width + 4, "", list->items[i]);
      free(list->items[i]);
    }
  }
  free(list->items);
}

int main(int argc, char *argv[]) {
  // --- Variable Definitions ---
  bool detach_mode = false;
  bool keep_stdin = false;
  bool allocate_ptty = false;
  bool read_only = false;
  bool remove_on_exit = false;
  char *container_name = NULL;
  char *restart_policy = "no";
  char *network = "bridge";
  char *host_name = NULL;
  char *work_dir = NULL;
  double ncpus = 0.0;
  long long memory = 0;
  long long mem_swap = 0;
  user_t user = {0};
  string_list_t published_ports = {0};
  string_list_t volumes = {0};
  string_list_t env_vars = {0};
  string_list_t caps_to_add = {0};
  string_list_t caps_to_drop = {0};

  // --- Flag Set Initialization ---
  flgset_t fset = {0};

  // --- Flag Definitions ---
  flgset_bool(&fset, &detach_mode, 'd', "detach",
              "Run container in background and print container ID");
  flgset_bool(&fset, &keep_stdin, 'i', "interactive",
              "Keep STDIN open even if not attached");
  flgset_bool(&fset, &allocate_ptty, 't', "tty", "Allocate a pseudo-TTY");
  flgset_bool(&fset, &remove_on_exit, 0, "rm",
              "Automatically remove the container when it exits");
  flgset_string(&fset, &container_name, 0, "name", "<NAME>",
                "Assign a name to the container");
  flgset_string(&fset, &restart_policy, 0, "restart", "<POLICY>",
                "Restart policy to apply when a container exits");
  flgset_string(&fset, &network, 0, "network", "<NET>",
                "Connect a container to a network");
  flgset_string(&fset, &host_name, 0, "hostname", "<NAME>",
                "Container host name");
  flgset_string(&fset, &work_dir, 'w', "workdir", "<PATH>",
                "Working directory inside the container");
  flgset_double(&fset, &ncpus, 0, "cpus", "<NUM>", "Number of CPUs");
  flgset_bool(&fset, &read_only, 0, "read-only",
              "Mount the container's root filesystem as read-only");

  // Corrected flgset_func calls with the parser function as the last argument
  flgset_func(&fset, &published_ports, 'p', "publish", "<HOST:CONT>",
              "Publish a container's port(s) to the host", list_parser);
  flgset_func(&fset, &volumes, 'v', "volume", "<HOST:CONT>",
              "Bind mount a volume", list_parser);
  flgset_func(&fset, &env_vars, 'e', "env", "<KEY=VAL>",
              "Set environment variables", list_parser);
  flgset_func(&fset, &memory, 0, "memory", "<SIZE>",
              "Memory limit (e.g., 512m, 1g)", mem_parser);
  flgset_func(&fset, &mem_swap, 0, "memory-swap", "<SIZE>",
              "Swap limit equal to memory plus swap", mem_parser);
  flgset_func(&fset, &user, 0, "user", "<UID:GID>",
              "Username or UID (format: <name|uid>[:<group|gid>])",
              user_parser);
  flgset_func(&fset, &caps_to_add, 0, "cap-add", "<CAP>",
              "Add Linux capabilities", list_parser);
  flgset_func(&fset, &caps_to_drop, 0, "cap-drop", "<CAP>",
              "Drop Linux capabilities", list_parser);

  // --- Parsing ---
  struct timespec start, end;
  timespec_get(&start, TIME_UTC);
  flgset_parse(&fset, argc, argv);
  timespec_get(&end, TIME_UTC);

  // --- Print Parsed Values (Beautifully) ---
  int label_width = 24;

  printf("--- Docker Run Emulation Results ---\n\n");
  printf("General:\n");
  printf("  %-*s %s\n", label_width,
         "Container Name:", container_name ? container_name : "(generated)");
  printf("  %-*s %s\n", label_width, "Detached Mode:", btoa(detach_mode));
  printf("  %-*s %s\n", label_width, "Interactive (-i):", btoa(keep_stdin));
  printf("  %-*s %s\n", label_width, "Pseudo-TTY (-t):", btoa(allocate_ptty));
  printf("  %-*s %s\n", label_width,
         "Remove on Exit (--rm):", btoa(remove_on_exit));
  printf("  %-*s %s\n", label_width, "Restart Policy:", restart_policy);
  printf("  %-*s %s\n", label_width,
         "Working Directory:", work_dir ? work_dir : "(default: /)");
  print_and_free_list("Environment Vars (-e):", &env_vars, label_width);
  printf("\n");

  printf("Resources:\n");
  printf("  %-*s %.2f\n", label_width, "CPU Limit:", ncpus);
  printf("  %-*s %lld bytes\n", label_width, "Memory Limit:", memory);
  printf("  %-*s %lld bytes\n", label_width, "Memory+Swap Limit:", mem_swap);
  printf("\n");

  printf("Network & Storage:\n");
  printf("  %-*s %s\n", label_width, "Network:", network);
  printf("  %-*s %s\n", label_width,
         "Hostname:", host_name ? host_name : "(generated)");
  print_and_free_list("Published Ports (-p):", &published_ports, label_width);
  print_and_free_list("Volumes (-v):", &volumes, label_width);
  printf("\n");

  printf("Security & Permissions:\n");
  printf("  %-*s %d:%d\n", label_width, "User (UID:GID):", user.uid, user.gid);
  printf("  %-*s %s\n", label_width, "Read-Only Root FS:", btoa(read_only));
  print_and_free_list("Capabilities to Add:", &caps_to_add, label_width);
  print_and_free_list("Capabilities to Drop:", &caps_to_drop, label_width);
  printf("\n----------------------------------\n");

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Flags parsed in %.9f seconds\n", elapsed);

  return 0;
}
