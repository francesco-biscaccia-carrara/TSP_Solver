#include "../include/algorithm.h"
#include "../include/tsp.h"
#include "../include/utils.h"

/// @brief check if the distance was computed before, if is not the case, compute and save the result
/// @param problem istance of the problem
/// @param i starting arc node
/// @param j ending arc node
/// @return the distance from i to j
double tsp_save_weight(instance * problem, int i, int j){
    if (i == j) return 0;

    if(!problem->edge_weights[coords_to_index(problem->nnodes,i,j)])
        problem->edge_weights[coords_to_index(problem->nnodes,i,j)] = euc_2d(&(problem->points[i]), &(problem->points[j]));
      
    return problem->edge_weights[coords_to_index(problem->nnodes,i,j)];
}

point_n_dist get_min_distance_point(int index, instance *problem, int* res) {

    double min = DBL_MAX;
    point_n_dist out = { .dist = 0.0, .index = 0};

    for(int i = 0; i < problem->nnodes; i++) {
        
        if(res[i] != 0) continue;               //if not assigned

        double dist = tsp_save_weight(problem,index,i);
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

    int* mem_check = calloc(problem->nnodes, sizeof(int));
    int* result = malloc(problem->nnodes * sizeof(int));

    result[0] = current_index;
    mem_check[index] = 1;

    for (int i = 1; i < problem->nnodes; i++) {  
        new_point = get_min_distance_point(result[i-1], problem, mem_check);
        
        cost += new_point.dist;
        mem_check[new_point.index] = 1;
        result[i] = new_point.index;
    }

    cost +=tsp_save_weight(problem, new_point.index, index);    
    free(mem_check);

    #if VERBOSE > 1
    printf("Partial \e[1mGREEDY\e[m solution starting from [%i]: \t%10.4f\n", index, cost);
    #endif


    if(cost < problem->result){
        free(problem->combination);

        problem->result = cost;
        problem->combination = result;
    }else{
        free(result);
    }
}

double check_cross(instance* problem,int i,int j){
    int k = (j+1 == problem->nnodes)? 0: j+1;
    return (tsp_save_weight(problem,i,j)+tsp_save_weight(problem,i+1,k))-(tsp_save_weight(problem,i,i+1)+tsp_save_weight(problem,j,k));
}

cross find_cross(instance* problem){
    cross best_cross = {-1,-1,DBL_MAX};

    for(int i=0;i<problem->nnodes;i++){
        for(int j=i+1;j<problem->nnodes;j++){
            if(j-i==1) continue;
            double delta_cost=check_cross(problem,problem->combination[i],problem->combination[j]);
            if(delta_cost < best_cross.delta_cost){
                best_cross.i = i;
                best_cross.j = j;
                best_cross.delta_cost = delta_cost;
            }
        }
    }
    //printf("BEST SWAP: %d %d %f\n",best_cross.i,best_cross.j,best_cross.delta_cost);
    return best_cross;
}


void tsp_g2opt(instance* problem){
  //TO DO
}