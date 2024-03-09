#include "tsp.h"
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

    printf("\nBSRESULT: %d\n",problem->best_sol->result);
    if(cost < problem->best_sol->result){
        printf("\nCULO\n");
        problem->best_sol->result = cost;
        problem->best_sol->combination = result;
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

int tsp_convert_coord_edge(uint32_t n,int i,int j){
    return i<j ? ((i*n-(i-1)*(i)/2) + (j-i-1)-i) : ((j*n-(j-1)*(j)/2) + (i-j-1)-j);
}

uint32_t tsp_save_weight(instance * inst, int i, int j){
    if (i == j) return 0;
    printf("\n\n__SGRODI: %d %d\n\n",i,j);

    if(!inst->edge_weights[tsp_convert_coord_edge(inst->nnodes,i,j)])
        inst->edge_weights[tsp_convert_coord_edge(inst->nnodes,i,j)] = euc_2d(&(inst->points[i]), &(inst->points[j]));
      
    return inst->edge_weights[tsp_convert_coord_edge(inst->nnodes,i,j)];
}
