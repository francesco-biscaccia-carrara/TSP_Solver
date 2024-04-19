#ifndef __TSP_EXACT_H 

#define __TSP_EXACT_H

#include "tsp_utils.h"
#include <ilcplex/cplex.h>


extern void     TSPCsolve(TSPinst*, TSPenv*);

extern void     TSPCbranchcut(TSPinst*,TSPenv*,CPXENVptr*,CPXLPptr*);
extern void     TSPCbenders(TSPinst*, TSPenv*,CPXENVptr*,CPXLPptr*);
extern void     patching(TSPinst*, int*, int*, const unsigned int);

#endif