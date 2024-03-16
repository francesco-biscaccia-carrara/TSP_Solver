#ifndef __LOAD_H 

#define __LOAD_H

#define VERBOSE	    2

#include "utils.h"

typedef struct {
    size_t nnodes;
    uint32_t random_seed;
    char file_name[60];
    char method[20];
    uint64_t time_limit;
} cli_info;

extern void parse_cli(int argc, char **argv, cli_info* cli_data);

#endif
