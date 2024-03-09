#include "tsp.h"
#include <math.h>

int euc_2d(point* a, point* b) {
    int dx = b->x - a->x;
    int dy = b->y - a->y; 

    return NINT(sqrt(SQUARE(dx) + SQUARE(dy)));
}

point_n_dist get_min_distance_point(int index, instance *problem, uint32_t* res) {

    uint32_t min = INFINITY, dist = 0;
    point_n_dist out;

    for(int i = 0; i < problem->nnodes; i++) {
        
        if(res[i] != -1) continue;                      //if not assigned
        dist = euc_2d(&(problem->points[index]), &(problem->points[i]));

        if (dist < min && dist != 0) {
            out.dist = dist;
            out.index = i;
            min = dist;
        }
    }

    return out;
}

void tsp_greedy(int index, instance* problem) {

    uint32_t cost = 0, current_index = index;
    point_n_dist new_point;

    //intialize solution
    uint32_t* result = malloc(5 * problem->nnodes);
    for(int i = 0; i < problem->nnodes; i++) result[i] = -1;


    for (int i = 0; i < problem->nnodes; i++) {  
        new_point = get_min_distance_point(current_index, problem, result);

        cost += new_point.dist;
        result[current_index] = (current_index == new_point.index) ? index : new_point.index;
        current_index = new_point.index;
    }

    if(VERBOSE > 8) {
        int p = index;
        for(int j = 0; j < problem->nnodes; j++) {
            printf("%i,", result[j]);
            //printf("%i->",result[p]);
            //p = result[p];
        }
        printf("\ncost: %i\n", cost);
    }

    if(cost < problem->best_cost){
        problem->best_cost = cost;
        problem->best_sol = result;
    }
}

/*void greedy_solution(int initial_node, instance *inst) {

    uint32_t* output = malloc(sizeof(uint32_t)*(inst->nnodes-1));
    for(int s = 0; s < inst->nnodes; s++) output[s] = -1;

    int cost = 0;

    long double min = INFINITY;
    point current_point = inst->points[initial_node];
    int index_current = initial_node;      //current index point
    int index_next = -1;        //next index point

    for (int i = 0; i < inst->nnodes; i++) {        //since each node have one output arc

        for (int j = 0; j < inst->nnodes; j++) {    
            
            if(output[j] != -1) continue;            //if is visited continue

            double dist = euc_2d(current_point, inst->points[j]);
            //printf("point[%i] -> point[%i] = %.2f \n", index_current, j, dist);

            if(dist < min && dist != 0) {
                min = dist;
                index_next = j;
            } 
        }
        cost += min;
        printf("%Lf\n", cost);
        output[index_current] = index_next;
        index_current = index_next;
        current_point = inst->points[index_current];
        min = INFINITY;
    }
    output[index_current] = initial_node;

    printf("%.2f\n", cost);
    if(cost < inst->best_cost) {
        inst->best_cost = cost;
        inst->best_sol = output;
    }
}*/