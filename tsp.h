#ifndef __TSP_H 

#define __TSP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERBOSE				  10 // Printing level  (=0 min output, =1 little output, =5 good output, =7 verbose, >=10 complete log)
#define INT_TOL		  		  1e-5 		
#define EPSILON		  		  1e-10	
#define MAX_TIME              8.64e+7	//A day

typedef struct {
    double x;
    double y;
} point;

typedef struct{

    uint32_t nnodes;
    uint32_t random_seed;

    point * points;
    uint64_t time_limit;
    uint64_t best_time;

    double * best_sol;
    double best_cost;
    char file_name[120];

} instance;
//This is a 56+120 Bytes aligned structure. Be careful when you modify it.

#endif