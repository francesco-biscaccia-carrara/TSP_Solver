#ifndef __TSP_SOLVER_H 

#define __TSP_SOLVER_H

#define MAX_DIST    10000
#define MAX_TIME    3.6e+6

#include "tsp_utils.h"

typedef struct{
    const TSPinst*      mt_inst;
    const int           mt_tabu_size; 
} mt_greedy_pars;

extern void     TSPsolve(TSPinst*, TSPenv*);

extern TSPsol   TSPgreedy(const TSPinst*, const unsigned int, void(const TSPinst*, int*, double*), char*);
extern void     TSPg2opt(const TSPinst*, int*, double*);
extern void     TSPg2optb(const TSPinst*, int*, double*);
extern TSPsol   TSPtabu(TSPinst*, const TSPenv*, const double);
extern TSPsol   TSPvns(TSPinst*, const TSPenv*, const double);

#endif