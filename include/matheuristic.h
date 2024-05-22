#ifndef __MATHEURISTIC_H 
#define __MATHEURISTIC_H

#include "tsp_exact.h"

extern void MATsolve(TSPinst*, TSPenv*);

extern void diving(TSPinst*, TSPenv*);
extern void local_branching(TSPinst*, TSPenv*);

#endif