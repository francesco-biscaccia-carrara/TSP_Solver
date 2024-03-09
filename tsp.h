#ifndef __TSP_H 

#define __TSP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define VERBOSE				  6 // Printing level  (=0 min output, =1 little output, =5 good output, =7 verbose, >=10 complete log)
#define INT_TOL		  		  1e-5 		
#define EPSILON		  		  1e-10	
#define MAX_TIME              8.64e+7	//A day

#define SQUARE(x)   (x*x)
#define NINT(x)     ((int) x + 0.5)

typedef struct {
    int result;
    int* combination;
} solution;

typedef struct {
    uint32_t dist, index;
} point_n_dist;

typedef struct {
    uint32_t x;
    uint32_t y;
} point;

typedef struct{

    size_t nnodes;
    uint32_t random_seed;

    point * points;
    int * edge_weights;
    uint32_t * best_sol;

    #if VERBOSE > 5
    uint64_t time_limit;
    uint64_t best_time;
    char file_name[120];
    #endif

    int best_cost;

} instance;

int euc_2d(point* a, point* b);
point_n_dist get_min_distance_point(point* p0, instance *problem, uint32_t* res);
void tsp_greedy(point* p0, int index, instance* problem);

#endif