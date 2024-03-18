#ifndef __ALGO_H 

#define __ALGO_H

#include "tsp.h"

#define EPSILON     1e-7
//#define INT_TOL		1e-5 	
#define SQUARE(x)   (x*x)
#define INDEX(n,i,j) ((i*n-(i-1)*(i)/2) + (j-i-1)-i)

typedef struct{
    int i,j;
    double delta_cost;
} cross;

typedef struct{
    uint32_t i,j,k,h;       // i->k     j->h
    double delta_cost;
} move;

typedef struct {
    double dist;
    uint32_t index;
} point_n_dist;

extern void solve_heuristic (cli_info* information, instance* problem);

extern void tsp_greedy(int index, instance* problem, void (opt_func)(int*, double*, instance*), char* opt_func_name);
//We could embed both in a single funcion and pass another pointer (a lot of redundance now)
extern void tsp_g2opt(int* tmp_sol, double* cost, instance* problem);
extern void tsp_g2opt_best(int* tmp_sol, double* cost, instance* problem);
void tabu_search(instance* problem, uint64_t initial_time, cli_info* cli_info);

#endif