#ifndef __TSP_H 

#define __TSP_H

#include "load.h"

#define MAX_DIST    10000

typedef struct {
    double x;
    double y;
} point;

typedef struct{
    size_t nnodes;
    uint32_t random_seed;

    point * points;
    double * edge_weights;

    double cost;
    int* solution;
} instance;

extern instance* instance_new();
extern instance* instance_new_cli(cli_info* cli_info);
extern void instance_delete(instance * inst);

extern double tsp_save_weight(instance * problem, int i, int j);
#endif