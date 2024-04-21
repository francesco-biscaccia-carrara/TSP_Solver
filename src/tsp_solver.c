#include "../include/tsp_solver.h"


/// @brief Solve an instance of TSP with an heuristic approach
/// @param inst instance of TSPinst
/// @param env instance of TSPenv
void TSPsolve(TSPinst* inst, TSPenv* env) {
    char* null_func[] = {"GREEDY", "TABU_R", "VNS"};
    char* optb_func[] = {"G2OPT_B", "TABU_B"};
    char* optf_func[] = {"G2OPT_F"};
    void* opt_func;

    if(strnin(env->method, null_func, 3)) { opt_func = NULL; }
    else if(strnin(env->method, optf_func, 1)) { opt_func = TSPg2opt; }
    else if(strnin(env->method, optb_func, 2)) { opt_func = TSPg2optb; }
    else { print_error("No function with alias"); }


    double init_time = get_time();
    for(int i = 0; i < inst->nnodes && time_elapsed(init_time) <= env->time_limit; i++) {
        TSPgreedy(inst, i, opt_func, env->method);
    }
    
    char* vns_func[] = {"VNS"};
    if(strnin(env->method, vns_func, 1)) { TSPvns(inst, env, init_time); }

    double final_time = get_time();

    #if VERBOSE > 0
		print_lifespan(final_time,init_time);
	#endif
}


/// @brief find a solution to TSP using gredy approach
/// @param inst instance of TSPinst
/// @param intial_node intial node
/// @param tsp_func improvement function
void TSPgreedy(TSPinst* inst, const unsigned int intial_node, void(tsp_func)(TSPinst*, int*, double*), char* func_name) {
    double cost = 0.0;
    int current_index = intial_node;

    int used_node[inst->nnodes];
    bzero(used_node, inst->nnodes * sizeof(used_node[0]));
    int result[inst->nnodes];

    result[0] = current_index;
    used_node[intial_node] = 1;

    for (int i = 1; i < inst->nnodes; i++) {  
        near_neighbor new_point = get_nearest_neighbor(inst, result[i-1], used_node);

        cost += new_point.dist;
        used_node[new_point.index] = 1;
        result[i] = new_point.index;
    }
    cost += get_arc(inst, result[inst->nnodes-1], intial_node);

    #if VERBOSE > 1
    printf("Partial \e[1m%7s\e[m solution starting from [%i]: \t%10.4f\n","GREEDY" , intial_node, cost);
    #endif

    if(tsp_func != NULL) {
        tsp_func(inst, result, &cost);

        #if VERBOSE > 1
        printf("Partial \e[1m%7s\e[m solution starting from [%i]: \t%10.4f\n", func_name, intial_node, cost);
        #endif
    }
    
    if(cost >= inst->cost) return;    
    instance_set_solution(inst, result, cost);
}


/// @brief execute G2Opt using first cross policy 
/// @param inst instance of TSPinst 
/// @param tour hamiltionian circuit
/// @param cost cost of path
void TSPg2opt(TSPinst* inst, int* tour, double* cost) {
    while (1) {
        cross curr_cross = find_first_cross(inst, tour);
        if(curr_cross.delta_cost >= -EPSILON) return;
        
        reverse(tour,curr_cross.i+1,curr_cross.j);
        *cost+=curr_cross.delta_cost;

        #if VERBOSE > 2
            check_tour_cost(inst, tour, *cost);
        #endif
    }
}


/// @brief execute G2Opt using best cross policy 
/// @param inst instance of TSPinst 
/// @param tour hamiltionian circuit
/// @param cost cost of path
void TSPg2optb(TSPinst* inst, int* tour, double* cost) {
    while (1) {
        cross curr_cross = find_best_cross(inst, tour);
        if(curr_cross.delta_cost >= -EPSILON) return;
        
        reverse(tour,curr_cross.i+1,curr_cross.j);
        *cost+=curr_cross.delta_cost;
            
        #if VERBOSE > 2
            check_tour_cost(inst, tour, *cost);
        #endif
    }
}


/// @brief execute VNS algorithm to improve TSP instance
/// @param inst instance of TSPinst 
/// @param env instance of TSPenv
/// @param init_time intiial times
void TSPvns(TSPinst* inst, const TSPenv* env, const double init_time) {
    double cost = inst->cost;
    int tmp_sol[inst->nnodes];
    memcpy(tmp_sol, inst->solution, inst->nnodes * sizeof(inst->solution[0]));

    while (time_elapsed(init_time) <= env->time_limit) {
        TSPg2optb(inst, tmp_sol, &cost);

        if(cost < inst->cost - EPSILON) {
            instance_set_solution(inst, tmp_sol, cost);
            
            #if VERBOSE > 0
            printf("new best cost:\t%10.4f\n", inst->cost);
            #endif
        }

        for(int i = 0; i < 4; i++) kick(tmp_sol, inst->nnodes);
        
        cost = 0;
        for (size_t i = 0; i < inst->nnodes - 1; i++) {
            cost += get_arc(inst, tmp_sol[i],tmp_sol[i+1]);
        }
        cost +=get_arc(inst, tmp_sol[inst->nnodes-1], tmp_sol[0]);
    }
}