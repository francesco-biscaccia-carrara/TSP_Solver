#ifndef __TSP_EXACT_H 

#define __TSP_EXACT_H

#include "tsp_eutils.h"

extern void     TSPCsolve(TSPinst*, TSPenv*);

extern TSPsol   TSPCbranchcut(TSPinst*,TSPenv*,CPXENVptr*,CPXLPptr*, const double);
extern TSPsol   TSPCbenders(TSPinst*, TSPenv*,CPXENVptr*,CPXLPptr*, const double);
extern void     patching(TSPinst*, int*, int*, const unsigned int, int*);

#endif