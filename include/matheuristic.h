#ifndef __MATHEURISTIC_H 
#define __MATHEURISTIC_H

#include "tsp_exact.h"

extern void MATsolve(TSPinst*, TSPenv*);

extern void diving(CPXENVptr*, CPXLPptr*, TSPinst*, TSPenv*);
//extern void local_branching();

#endif