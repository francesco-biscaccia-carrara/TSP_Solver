#ifndef __TSP_EXACT_H 

#define __TSP_EXACT_H

#include "tsp_utils.h"
#include <ilcplex/cplex.h>


extern int      tsp_CPX_opt(TSPinst*);
extern void     tsp_bender_loop(TSPinst*, TSPenv*);
extern void     patching(TSPinst*, int*, int*);

#endif