#include "../include/algorithm.h"
#include "../include/tsp.h"

double tsp_save_weight(instance * problem, int i, int j){
    if (i == j) return 0;
    if(!problem->edge_weights[coords_to_index(problem->nnodes,i,j)])
        problem->edge_weights[coords_to_index(problem->nnodes,i,j)] = euc_2d(&(problem->points[i]), &(problem->points[j]));
      
    return problem->edge_weights[coords_to_index(problem->nnodes,i,j)];
}

point_n_dist get_min_distance_point(int index, instance *problem, int* res) {

    double min = DBL_MAX, dist = 0;
    point_n_dist out = { .dist = 0.0, .index = 0};

    for(int i = 0; i < problem->nnodes; i++) {
        
        if(res[i] != -1) continue; //if not assigned
        dist = tsp_save_weight(problem,index,i);

        if (dist < min && dist != 0) {
            out.dist = dist;
            out.index = i;
            min = dist;
        }
    }

    return out;
}

void tsp_greedy(int index, instance* problem) {

    double cost = 0;
    int current_index = index;
    point_n_dist new_point;

    //printf("BSOL IN GREEDY: %u\n", problem->result);

    //intialize solution
    int* result = malloc(sizeof(int)* problem->nnodes);
    for(int i = 0; i < problem->nnodes; i++) result[i] = -1;

    for (int i = 0; i < problem->nnodes; i++) {  
        new_point = get_min_distance_point(current_index, problem, result);

        cost += new_point.dist;
        result[current_index] = (current_index == new_point.index) ? index : new_point.index;
        current_index = new_point.index;
    }

    #if VERBOSE > 8
    printf("__log: BS Combination: ");
    for(int j = 0; j < problem->nnodes; j++) {
        printf("| %d", result[j]);
    }
        printf("|\n");
        printf("__log:BS Cost: %10.4f\n", cost);
    #endif

    if(cost < problem->result){
        free(problem->combination);

        problem->result = cost;
        problem->combination = result;
    }else{
        free(result);
    }
}
