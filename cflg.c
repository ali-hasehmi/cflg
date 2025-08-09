#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef CFLG_MAP_SIZE
#define CFLG_MAP_SIZE 32
#endif

typedef 
enum {
    Bool,
    Int,
    String,
    Float,
    UInt,
    Int64,
    Uint64,
    Double,
    UserDefined,
} flagtype_t;


typedef
struct {
    flagtype_t type;
    void *dest;	 
    char *usage;
    char name;
    char *name_long;
} flag_t;

typedef 
struct {
    uint32_t (*hashFunc)(const char*,size_t);
    size_t len;
    size_t cap;
    flag_t **map;
} map_t;

typedef
struct {
    bool parsed;	
    int narg; 		// number of arguments remaining after flags have been processed.
    char **args;	// non-flag arguments after flags have beeen processed.
    map_t flags;    
} cflg_flagset_t;


int cflg_parse(int argc, char *argv[]);
int cflg_flagset_parse(cflg_flagset_t* flgset, int argc, char *argv[]);


bool map_insert(map_t *m, const char* k,size_t len,flag_t* v);
flag_t* map_find(map_t *m, const char* k, size_t len);

#define PARSE_ARG_REMAINED    0 
#define PARSE_ARG_CONSUMED    1
#define PARSE_ARG_NEEDED     -1
#define PARSE_ARG_INVALID    -2

int parse_bool(flag_t *f,const char *arg);
int parse_int(flag_t *f,const char *arg);
int parse_uint(flag_t *f,const char *arg);
int parse_float(flag_t *f,const char *arg);
int parse_string(flag_t *f,const char *arg);

int (*parse_handlers[])(flag_t *,const char *arg)= {
    [Bool]   = parse_bool,
    [Int]    = parse_int,
    [UInt]   = parse_uint,
    [Float]  = parse_float,
    [String] = parse_string,
};

void parseflg_long(cflg_flagset_t *flgset, char* first, char *second);
void parseflg(cflg_flagset_t *fset, char *first, char *second);

int cflg_flagset_parse(cflg_flagset_t* fset, int argc, char *argv[]) {

	if (fset->parsed) return 0;
    
	fset->narg = 0;
	fset->args = (char **) malloc(argc * sizeof(char*));
	if (!fset->args) return -1;
	int i = 0;
	for( ; i < argc; ++i){

	    int len = strlen(argv[i]);

	    // if not a flag, add it to args and continue
	    if(len < 2 || argv[i][0] != '-'){
		fset->args[fset->narg] = argv[i];
		fset->narg++;
		continue;
	    }

	    // if it's a long flag
	    if(argv[i][1] == '-') {
		if(len == 2) break; // -- => stop parsing flags
	    
		// search for '=' in flag
		bool skip_next = false;
		char *arg = strchr(argv[i],'=');
		char *flag;
		size_t flag_len;
		if(arg == NULL) /* no '=' in flag */ {
		    arg = argv[i+1]; 
		    flag = argv[i] + 2;
		    flag_len = len - 2;
		    skip_next = true;
		} else {
		    flag = argv[i] + 2;
		    flag_len = arg - flag ;
		    arg++;
		}	
		flag_t *f = map_find(&fset->flags, flag, flag_len);
		if(f == NULL){
		    // TODO: handle invalid flag error
		}	
		int res = parse_handlers[f->type](f, arg);
		switch(res){
		    case PARSE_ARG_CONSUMED:
			if( skip_next ){
			    i++;
			}
		    break;
		    case PARSE_ARG_REMAINED:
			break;
		default:
		    // TODO: handle argument flag errors
		    
		}
	    }
	    // if it's a short flag
	    else {
		// iterate over all short flags
		for(int j = 1; j < len; ++j) {
		    char *flag = argv[i] + j;
		    flag_t *f = map_find(&fset->flags, flag ,1);
		    if (f == NULL){
			// TODO: Handle invalid flag error
		    }
		    bool skip_next = false;
		    char *arg =  argv[i] + j + 1;
		    if( j + 1 == len){ // TODO: OR "*arg == '\0'" which one is better? 
			arg = argv[i+1];
			skip_next = true;
		    }
		    int res = parse_handlers[f->type](f, argv[i] + j + 1);
		    bool break_loop =false;
		    switch(res){
			case PARSE_ARG_CONSUMED:
			    break_loop = true;
			    if( skip_next ){
				i++;
			    }
			    break;
			case PARSE_ARG_REMAINED:
			    break;
			default:
			// TODO: handle argument flag errors
		    
		    }
		    if (break_loop) break;
		}
	    }
	}

	for(i = i + 1 ; i < argc; ++i){
	 	fset->args[fset->narg] = argv[i];
		fset->narg++;
	}

	fset->parsed = true;
	return 0;
}


void parseflg(cflg_flagset_t *flgset, char *first, char *second) {
    for(int i = 1 ; i < strlen(first) ; ++i){
    }
}

bool map_insert(map_t *m, const char* key, size_t len, flag_t* v) {
    
    // panic if key is NULL
    assert(key != NULL);

    // if map is full, resize the backing array
    if(m->cap <= m->len) {
	m->map = realloc(m->map, (m->cap * 2 * sizeof(flag_t*)));
	if(m->map == NULL) return false;
	memset(m->map + m->cap ,NULL, m->cap);
	m->cap *= 2;
    }

    // hash the key and find the location 
    uint32_t loc = m->hashFunc(key,len) % m->cap;

    // find an empty bucket
    while(m->map[loc] != NULL) {
	// check if key has been inserted before
	if  ((len == 1 &&
	    m->map[loc]->name == key[0]) ||
	    ( m->map[loc]->name_long != NULL &&
	    !strncmp(m->map[loc]->name_long, key, len))) 
	{
	    return false;
	}
	loc = (loc+1) % m->cap;
    }

    m->map[loc] = v;
    m->len++;
}

flag_t* map_find(map_t *m, const char* key, size_t len) {

    // panic if key is NULL
    assert(key != NULL);

    // return if map is empty
    if (m->len == 0){
	return NULL;
    }
    
    uint32_t loc = m->hashFunc(key,len) % m->cap;
    while(m->map[loc] != NULL){
	// check if key has been inserted before
	if  ((len == 1 &&
	    m->map[loc]->name == key[0]) ||
	    ( m->map[loc]->name_long != NULL &&
	    !strncmp(m->map[loc]->name_long, key, len))) 
	{
	    return m->map[loc];
	}
	loc = (loc+1) % m->cap;
    }
    return NULL;
}


int parse_bool(flag_t *f, const char *arg) {
    // BUG: multiple calls, causes different results
    // TODO: arbitrary number of calls, must only result in reverse of default value
   *(bool*)f->dest = !(*(bool*)f->dest);
    return PARSE_ARG_REMAINED;
}

int parse_int(flag_t *f,const char *arg) {
    // TODO: instead of "strlen(arg) == 0" use arg[0] == '\0'
    if ( arg == NULL || strlen(arg) == 0){
	return PARSE_ARG_NEEDED;
    }

    char *endptr;
    long int n = strtol(arg, &endptr, 0);
    if ( *endptr != '\0' ){
	return PARSE_ARG_INVALID;
    }

    *(int*)f->dest = n;

    return PARSE_ARG_CONSUMED;
}

int parse_uint(flag_t *f,const char *arg) {
    if ( arg == NULL || strlen(arg) == 0){
	return PARSE_ARG_NEEDED;
    }

    char *endptr;
    unsigned long int n = strtoul(arg, &endptr, 0);
    if ( *endptr != '\0' ){
	return PARSE_ARG_INVALID;
    }

    *(unsigned int*)f->dest = n;

    return PARSE_ARG_CONSUMED;
}

int parse_float(flag_t *f,const char *arg) {
    if ( arg == NULL  || strlen(arg) == 0 ){
	return PARSE_ARG_NEEDED;
    }

    char *endptr;
    float n = strtof(arg, &endptr);
    if ( *endptr != '\0' ){
	return PARSE_ARG_INVALID;
    }

    *(float*)f->dest = n;

    return PARSE_ARG_CONSUMED;
}

int parse_string(flag_t *f,const char *arg) {
    if ( arg == NULL || strlen(arg) == 0){
	return PARSE_ARG_NEEDED;
    }

    *(char**)f->dest = (char *)arg;
}


