#ifndef __TSP_H 

#define __TSP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define VERBOSE	    0 // Printing level  (=0 min output, =1 little output, =5 good output, =7 verbose, >=10 complete log)
#define INT_TOL		1e-5 		
#define EPSILON     1e-10	
#define MAX_DIST    10000

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

    uint32_t result;
    int* combination;

    #if VERBOSE > 5
    char file_name[120];
    #endif

} instance;

instance* instance_new();
void instance_delete(instance * inst);

extern void tsp_greedy(int index, instance* problem);
extern void tsp_instance_from_cli(int argc, char** argv,instance* problem);
extern void tsp_plot(instance* problem);

extern void print_best_solution_info(instance* problem);
#endif