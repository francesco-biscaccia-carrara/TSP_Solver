#ifndef __TSP_SOLVER_H 

#define __TSP_SOLVER_H

#define MAX_DIST    10000
#define MAX_TIME    3.6e+6

#include "tsp_utils.h"

typedef struct{
    const TSPinst*      mt_inst;
    void*               mt_opt_fun;
    double              mt_init_time;
    TSPenv*             mt_env;
    TSPsol*             mt_greedy_sol; 
} mt_greedy_pars;

extern void     TSPsolve(TSPinst*, TSPenv*);

extern TSPsol   TSPgreedy(const TSPinst*, const TSPenv*, const unsigned int, void(const TSPinst*,const TSPenv*,double, int*, double*), char*, double);
extern void     TSPg2opt(const TSPinst*,const TSPenv*,double, int*, double*);
extern void     TSPg2optb(const TSPinst*, const TSPenv*,double, int*, double*);
extern TSPsol   TSPtabu(TSPinst*, const TSPenv*, const double);
extern TSPsol   TSPvns(TSPinst*, const TSPenv*, const double);

#endif