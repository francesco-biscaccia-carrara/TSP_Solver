#ifndef __ALGO_H 

#define __ALGO_H

#include "tsp.h"

extern void solve_heuristic (cli_info* information, instance* problem);
void tsp_greedy(int index, instance* problem, void (opt_func)(int*, double*, instance*), char* opt_func_name);
extern void tsp_g2opt(int* tmp_sol, double* cost, instance* problem);
#endif