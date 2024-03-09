#include "../include/tsp.h"
#include <math.h>

int euc_2d(point* a, point* b) {
    int dx = b->x - a->x;
    int dy = b->y - a->y; 

    return NINT(sqrt(SQUARE(dx) + SQUARE(dy)));
}

point_n_dist get_min_distance_point(int index, instance *problem, uint32_t* res) {

    uint32_t min = UINT32_MAX, dist = 0;
    point_n_dist out;

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

    uint32_t cost = 0, current_index = index;
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

    if(VERBOSE > 8) {
        int p = index;
        for(int j = 0; j < problem->nnodes; j++) {
            printf("%i,", result[j]);
            //printf("%i->",result[p]);
            //p = result[p];
        }
        printf("\ncost: %i\n", cost);
    }

    //printf("\nBSRESULT: %u\n",problem->result);
    if(cost < problem->result){
    //    printf("\nCULO\n");
        problem->result = cost;
        problem->combination = result;
    }
}

int tsp_convert_coord_edge(uint32_t n,int i,int j){
    return i<j ? ((i*n-(i-1)*(i)/2) + (j-i-1)-i) : ((j*n-(j-1)*(j)/2) + (i-j-1)-j);
}

uint32_t tsp_save_weight(instance * inst, int i, int j){
    if (i == j) return 0;
    //printf("\n\n__SGRODI: %d %d\n\n",i,j);

    if(!inst->edge_weights[tsp_convert_coord_edge(inst->nnodes,i,j)])
        inst->edge_weights[tsp_convert_coord_edge(inst->nnodes,i,j)] = euc_2d(&(inst->points[i]), &(inst->points[j]));
      
    return inst->edge_weights[tsp_convert_coord_edge(inst->nnodes,i,j)];
}
