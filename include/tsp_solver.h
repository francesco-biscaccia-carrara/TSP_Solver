#ifndef __TSP_SOLVER_H 

#define __TSP_SOLVER_H

#define MAX_DIST    10000
#define MAX_TIME    3.6e+6

#include "tsp_utils.h"

extern void     TSPsolve(TSPinst*, TSPenv*);

extern TSPsol   TSPgreedy(const TSPinst*, const unsigned int, void(const TSPinst*, int*, double*), char*);
extern void     TSPg2opt(const TSPinst*, int*, double*);
extern void     TSPg2optb(const TSPinst*, int*, double*);
extern void     TSPtabu(TSPinst*, const TSPenv*, const double);
extern void     TSPvns(TSPinst*, const TSPenv*, const double);

#endif