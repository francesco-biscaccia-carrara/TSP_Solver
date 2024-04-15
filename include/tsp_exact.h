#ifndef __TSP_EXACT_H 

#define __TSP_EXACT_H

#include "tsp_utils.h"
#include <ilcplex/cplex.h>


extern void     TSPCsolve(TSPinst*, TSPenv*);

extern int      tsp_CPX_opt(TSPinst*);
extern void     tsp_bender_loop(TSPinst*, TSPenv*, const unsigned int);
extern void     patching(TSPinst*, int*, int*, const unsigned int);

#endif