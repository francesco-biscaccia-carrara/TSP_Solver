#ifndef __TSP_MATHUTILS_H 

#define __TSP_MATHUTILS_H

#include "tsp_exact.h"

extern enum { Random, Weighted, Probably } FIX_STRATEGY;

extern int      arc_to_fix(int, int*, TSPinst*, int, int);
extern void     fix_to_model(CPXENVptr, CPXLPptr, int*, int);
extern void     unfix_to_model(CPXENVptr, CPXLPptr, int*, int);

extern void     local_tour_costraint();

#endif