#ifndef __LOAD_H 

#define __LOAD_H

#define VERBOSE	    2

#include "utils.h"

typedef struct {
    size_t nnodes;
    uint32_t random_seed;
    char file_name[64];
    char method[23];
    char mt;
    uint64_t time_limit;
} cli_info;

extern cli_info global_cli;
extern void parse_cli(int argc, char **argv, cli_info* cli_data);

#endif
