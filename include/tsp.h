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

    uint64_t time_limit;
    uint32_t result;
    int* combination;

    #if VERBOSE > 5
    char file_name[120];
    #endif

} instance;

instance* instance_new();
void instance_delete(instance * inst);

extern int euc_2d(point* a, point* b);
extern point_n_dist get_min_distance_point(int index, instance *problem, uint32_t* res);
extern void tsp_greedy(int index, instance* problem);

extern uint32_t tsp_save_weight(instance * inst, int i, int j);//tmp
extern int tsp_convert_coord_edge(uint32_t n,int i,int j); //tmp
#endif