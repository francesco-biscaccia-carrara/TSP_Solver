#ifndef __TSP_H 

#define __TSP_H

#include "utils.h"

#define VERBOSE	    0
#define INT_TOL		1e-5 		
#define EPSILON     1e-10	
#define MAX_DIST    10000

typedef struct {
    double dist;
    uint32_t index;
} point_n_dist;

typedef struct{
    size_t nnodes;
    uint32_t random_seed;

    point * points;
    double * edge_weights;

    double result;
    int* combination;
} instance;

extern instance* instance_new();
extern void instance_delete(instance * inst);

extern void tsp_instance_from_cli(instance *problem, cli_info* cli);

#endif