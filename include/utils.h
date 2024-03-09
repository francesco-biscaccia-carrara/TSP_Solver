#ifndef __UTILS_H 

#define __UTILS_H

#include "tsp.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>  
#include <float.h> 
#include <time.h>

extern void print_error(const char *err);
extern uint64_t get_time();
extern void help_info();
extern int coords_to_index(uint32_t n,int i,int j);
extern int euc_2d(point* a, point* b);
#endif