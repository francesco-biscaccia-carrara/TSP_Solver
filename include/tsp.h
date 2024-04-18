#ifndef __TSP_H 

#define __TSP_H

#define MAX_DIST    10000
#define MAX_TIME    3.6e+6
#define VERBOSE	    2

#include "utils.h"

typedef struct {
    double x;
    double y;
} point;

typedef struct{
    unsigned int    nnodes;
    unsigned int    random_seed;
    point *         points;
    double *        edge_weights;
    double          cost;
    int*            solution;
} TSPinst;

typedef struct{
    unsigned int    nnodes;
    unsigned int    random_seed;
    char*           file_name;
    char*           method;
    char            mt;
    uint64_t        time_limit;
} TSPenv;

//TSPinst functions
extern TSPinst* instance_new();
extern TSPinst* instance_new_env(TSPenv*);
extern void     instance_delete(TSPinst*);
extern void     instance_set_solution(TSPinst*, const int*, const double);

//TSPenv functions
extern TSPenv*  environment_new();
extern TSPenv*  environment_new_cli(char**, const int);
extern void     environment_delete(TSPenv*);
extern void     environment_set_method(TSPenv*, char*);
extern void     environment_set_seed(TSPenv*, const unsigned int);

#endif