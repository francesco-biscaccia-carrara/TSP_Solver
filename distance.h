#ifndef _DISTANCE_H_

#define _DISTANCE_H_

#include "tsp.h"
#include "utils.h"

int euc_2d (point a, point b);
int man_2d (point a, point b);
int max_2d (point a, point b);
//int ceil_2d (point a, point b);

inline int nint(double x) { return ((int)(x+0.5)); }
#endif