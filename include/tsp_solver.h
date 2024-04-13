#ifndef __TSP_SOLVER_H 

#define __TSP_SOLVER_H

#define MAX_DIST    10000
#define MAX_TIME    3.6e+6

#include "tsp_utils.h"

extern void     TSPsolve(TSPinst*, TSPenv*);

extern void     TSPgreedy(TSPinst*, const unsigned int, void(TSPinst*, int*, double*), char*);
extern void     TSPg2opt(TSPinst*, int*, double*);
extern void     TSPg2optb(TSPinst*, int*, double*);
extern void     TSPvns(TSPinst*, const TSPenv*, const double);

#endif