#ifndef __TSP_H 

#define __TSP_H

#define MAX_DIST    10000
#define MAX_TIME    3.6e+6
#define VERBOSE	    0

#include "utils.h"
#define REMAIN_TIME(init_time, env) (time_elapsed(init_time) <= env->time_limit)

typedef struct {
    double x;
    double y;
} point;

typedef struct {
    double cost;
    int*   tour;
} TSPsol;

typedef struct{
    unsigned int    nnodes;
    unsigned int    random_seed;
    point *         points;
    double          cost;
    int*            solution;
} TSPinst;

typedef struct{
    unsigned int    nnodes;
    unsigned int    random_seed;
    char*           file_name;
    char*           method;
    char            warm;
    char            perf_v;
    double          time_exec;
    uint64_t        time_limit;
    int             tabu_par;
    int             vns_par;
} TSPenv;

extern double* edge_weights;

//TSPinst functions
extern TSPinst* instance_new();
extern TSPinst* instance_new_env(TSPenv*);
extern void     instance_delete(TSPinst*);
extern void     instance_set_solution(TSPinst*, const int*, const double);
extern void     instance_set_best_sol(TSPinst*, const TSPsol);

//TSPenv functions
extern TSPenv*  environment_new();
extern TSPenv*  environment_new_cli(char**, const int);
extern void     environment_delete(TSPenv*);
extern void     environment_set_method(TSPenv*, char*);
extern void     environment_set_seed(TSPenv*, const unsigned int);

#endif