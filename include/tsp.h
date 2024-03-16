#ifndef __TSP_H 

#define __TSP_H

#include "utils.h"

#define VERBOSE	    2
#define INT_TOL		1e-5 		
#define EPSILON     1e-7	
#define MAX_DIST    10000

typedef struct{
    size_t nnodes;
    uint32_t random_seed;

    point * points;
    double * edge_weights;

    double result;
    int* combination;
} instance;

extern instance* instance_new();
extern instance* instance_new_cli(cli_info* cli_info);

extern void instance_delete(instance * inst);
#endif