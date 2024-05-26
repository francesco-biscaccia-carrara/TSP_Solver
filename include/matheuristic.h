#ifndef __MATHEURISTIC_H 
#define __MATHEURISTIC_H

#include "tsp_mathutils.h"

extern void MATsolve(TSPinst*, TSPenv*);

extern void diving(int, CPXENVptr, CPXLPptr, TSPinst*, TSPenv*, const double);
extern void local_branching(CPXENVptr, CPXLPptr, TSPinst*, TSPenv*, const double);

#endif