#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>

#define CFLG_IMPLEMENTATION
#include "../cflg.h"

/*
 *
 *
 *
 * docker run -dit \
 * -p 8080:80 -p 443:443 \
 * -v /host/data:/data -v /logs:/var/log/nginx \
 * --name=mynginx \
 * --restart=on-failure:5 \
 * --env=DEBUG=true --env=APP_ENV=production \
 * --cpus=2.5 --memory=1g --memory-swap=2g \
 * --network=my-net --hostname=myhost.local \
 * --user=1000:1000 \
 * --workdir=/app \
 * --cap-add=NET_ADMIN --cap-drop=SYS_PTRACE \
 * --health-cmd="curl -f http://localhost/ || exit 1" \
 * --health-interval=30s --health-retries=3 \
 * --read-only --rm \
 * nginx:latest /bin/bash -c "echo hello && tail -f /var/log/nginx/access.log"
 *
 *
 *
 */

#define btoa(a) ( (a) ? "true" : "false" )

typedef struct {
  struct {
    uint16_t host;
    uint16_t container;
  } publish_port[16];
  size_t len;
} publish_ports_t;
;

typedef struct {
  uint16_t uid;
  uint16_t gid;
} user_t;

int mem_parser(flg_t* f, const char* arg) {
    if (arg == NULL || *arg == '\0') {
        return CFLG_PARSE_ARG_NEEDED;
    }

    size_t len = strlen(arg);
    if ( len < 2 ) return CFLG_PARSE_ARG_INVALID;
    
    int64_t multi = 1;
    switch(arg[len-1]) {
        case 'k': case 'K':
            multi = 1024;
        break;

        case 'm': case 'M':
            multi = 1024 * 1024;
        break;

        case 'g': case 'G':
            multi = 1024 * 1024 * 1024;
        break; 
        default: 
            return CFLG_PARSE_ARG_INVALID;
    }

    char *endptr;
    int64_t val = strtol(arg, &endptr, 10);
    if (endptr != arg + len - 1) return CFLG_PARSE_ARG_INVALID;
    
    *(int64_t*)(f->dest) = val * multi;
    return CFLG_PARSE_ARG_CONSUMED;
}

int user_parser(flg_t* f, const char* arg) {
    if (arg == NULL || *arg == '\0') {
        return CFLG_PARSE_ARG_NEEDED;
    }

    char *del = strchr(arg, ':');
    if (del == NULL ) {
        return CFLG_PARSE_ARG_INVALID;
    }
    *del = '\0';
    del++;

    char *endptr;
    int uid = strtoul(arg, &endptr, 10);
    if (*endptr != '\0') {
        *(del-1) = ':';
        return CFLG_PARSE_ARG_INVALID;
    }
   
    int gid = strtol(del, &endptr, 10);
    if (*endptr != '\0') {
        *(del-1) = ':';
        return CFLG_PARSE_ARG_INVALID;
    }
    
    ((user_t*)f->dest)->uid = uid;
    ((user_t*)f->dest)->gid = gid;
    *(del-1) = ':';
    return CFLG_PARSE_ARG_CONSUMED;
}

int publish_ports_parser(flg_t* f, const char* arg) {
    if (arg == NULL || *arg == '\0') {
        return CFLG_PARSE_ARG_NEEDED;
    }

    char *del = strchr(arg, ':');
    if (del == NULL  || *(del+1) == '\0') {
        return CFLG_PARSE_ARG_INVALID;
    }
    *del = '\0';
    del++;

    char *endptr;
    int host_port = strtoul(arg, &endptr, 10);
    if (*endptr != '\0') {
        *(del-1) = ':';
        return CFLG_PARSE_ARG_INVALID;
    }
   
    int container_port = strtol(del, &endptr, 10);
    if (*endptr != '\0') {
        *(del-1) = ':';
        return CFLG_PARSE_ARG_INVALID;
    }
    
    publish_ports_t* temp = (publish_ports_t*)f->dest;
    temp->publish_port[temp->len].host = host_port;
    temp->publish_port[temp->len].container = container_port;
    temp->len++;

    *(del-1) = ':';
    return CFLG_PARSE_ARG_CONSUMED;
}

void printPPs(publish_ports_t* pps) {
    printf("pps: {");
    for (int i = 0; i < pps->len ; ++i) {
        printf("\n      {.host: %hu , .container: %hu},\n",pps->publish_port[i].host, pps->publish_port[i].container);
    }
    printf("}\n");
}
int main(int argc, char *argv[]) {

  bool keep_stdin = false;
  bool allocate_ptty = false;
  bool detach_mode = false;
  bool read_only = false;
  char *contianer_name = "RandomName";
  char *network = "bridge";
  char *host_name = "RandomContainerID";
  char *work_dir = "/";
  float ncpus = -1;    // default: use all host CPUs
  int64_t memory = -1; // default use all host memory
  int64_t mem_swap = -1;
  user_t user = {0};         // default root(0:0)
  publish_ports_t pps = {0}; // default none

  flgset_t fset = {0}; // IMPORTANT: always initialize it with zero

  flgset_bool(&fset, &keep_stdin, 'i', NULL,
              "Keep STDIN open even if not attached.");
  flgset_bool(&fset, &allocate_ptty, 't', NULL,
              "Allocate a pseudo-TTY for interactive use.");
  flgset_bool(&fset, &detach_mode, 'd', NULL,
              "Run the container in detached (background) mode.");
  flgset_bool(&fset, &read_only, 0, "read-only",
              "mount the container’s root filesystem as read-only.");
  flgset_string(&fset, &contianer_name, 0, "name", NULL,
                "Assign the container the explicit name");
  flgset_string(&fset, &network, 0, "network", NULL,
                "Connect the container to the custom Docker network");
  flgset_string(&fset, &host_name, 0, "hostname", NULL,
                "Set the container hostname");
  flgset_string(&fset, &work_dir, 0, "workdir", NULL,
                "Assign the container the explicit name");
  flgset_float(&fset, &ncpus, 0, "cpus", "NUM",
               "Limit the container to using at most NUM CPUs");
  flgset_func(&fset, &memory, 0, "memory", "SIZE",
              "Limit container memory usage to SIZE", mem_parser);
  flgset_func(&fset, &mem_swap, 0, "memory-swap", "SIZE",
              "Allow up to SIZE including swap memory.", mem_parser);
  flgset_func(&fset, &user, 0, "user", "UID:GID",
              "Allow up to SIZE including swap memory.", user_parser);
  flgset_func(&fset, &pps, 'p', NULL, "HOST:CONTAINER",
              "MAP HOST port to CONTAINER port", publish_ports_parser);

  flgset_parse(&fset, argc, argv);

    printf("keep open stdin: %s\n"
           "allocate_ptty  : %s\n"
           "detach_mode    : %s\n"
           "read_only      : %s\n"
           "contianer name : %s\n"
           "network        : %s\n"
           "host_name      : %s\n"
           "work_dir       : %s\n"
           "ncpus          : %f\n"
           "memory         : %ld bits\n" 
           "mem_swap       : %ld bits\n"
           "user.uid       : %hu\n"    
           "user.gid       : %hu\n",
           btoa(keep_stdin),
           btoa(allocate_ptty),
           btoa(detach_mode),
           btoa(read_only),
           contianer_name,
           network,
           host_name,
           work_dir,
           ncpus,
           memory,
           mem_swap,
           user.uid,
           user.gid
           );

        printPPs(&pps);
  return 0;
}
