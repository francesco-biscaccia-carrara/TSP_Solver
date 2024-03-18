#ifndef __UTILS_H 

#define __UTILS_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>  
#include <float.h> 
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>

extern void print_error(const char *err);
extern double time_elapsed(double intial_time);
extern double get_time();
extern void save_cost_on_file(size_t nnodes,uint32_t seed,double cost);
#endif