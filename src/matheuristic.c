#include "../include/matheuristic.h"

static inline void CPLEX_sol_from_inst(const unsigned int nnodes, const int* solution,int* index, double* value) {
		for(int i = 0; i < nnodes-1; i++){
			index[i] = coords_to_index(nnodes,solution[i],solution[i+1]);
			value[i] = 1.0;
		}
		index[nnodes-1] = coords_to_index(nnodes,solution[nnodes-1],solution[0]);
		value[nnodes-1] = 1.0;
}

void MATsolve(TSPinst* inst, TSPenv* env) {
    CPXENVptr CPLEX_env = NULL; 
	CPXLPptr CPLEX_lp = NULL;
	CPLEX_model_new(inst, &CPLEX_env, &CPLEX_lp);    

    if(!strncmp(env->method,"DIVING", 6)) { diving(&CPLEX_env, &CPLEX_lp, inst, env); }
    CPLEX_model_delete(&CPLEX_env,&CPLEX_lp);
}

int fix_arc(int* block_edge, int* solution, int p, int nnodes) {
    int k = 0;
    for(int i = 0; i < nnodes-1; i++) {
        
        if(rand()%10 < p) continue;
        if(i == nnodes-1) {
            block_edge[k++] = coords_to_index(nnodes, solution[nnodes-1], solution[0]);
        }
        else block_edge[k++] = coords_to_index(nnodes, solution[i], solution[i+1]);
    }
    return k;
}

void fix_to_model(CPXENVptr* env, CPXLPptr* lp, int* variable_to_fix, int variable_to_fix_size) {
    char L = 'L';
    double v = 1.0;
    for(int i = 0; i < variable_to_fix_size; i++) {
        CPXchgbds(*env, *lp, 1, variable_to_fix+i, &L, &v);
    }
}

void remove_fix(CPXENVptr* env, CPXLPptr* lp, int* variable_to_fix, int variable_to_fix_size) {
    char L = 'L';
    double v = 0.0;
    for(int i = 0; i < variable_to_fix_size; i++) {
        CPXchgbds(*env, *lp, 1, variable_to_fix+i, &L, &v);
    }
}

void diving(CPXENVptr* CPLEX_envs, CPXLPptr* CPLEX_lps, TSPinst* inst, TSPenv* env) {    

    TSPsol sol = TSPgreedy(inst, rand()%inst->nnodes, NULL, "");   
    instance_set_solution(inst, sol.tour, sol.cost);

    CPXENVptr CPLEX_env = NULL; 
    CPXLPptr CPLEX_lp = NULL;
    CPLEX_model_new(inst, &CPLEX_env, &CPLEX_lp);
    CPLEX_post_heur(&CPLEX_env, &CPLEX_lp, inst->solution, inst->nnodes);

    int* x = calloc(inst->nnodes, sizeof(int));
    int x_size = 0;
    double start_time = get_time();

    while(time_elapsed(start_time) < env->time_limit) {
        int percfix = (rand()%3)+1;

        #if VERBOSE > 0
            print_state(Info, "fix %i%% of the variables\n", percfix*10);
        #endif

        x_size = fix_arc(x, inst->solution, percfix, inst->nnodes);
        fix_to_model(&CPLEX_env, &CPLEX_lp, x, x_size);

        TSPCbranchcut(inst, env, &CPLEX_env, &CPLEX_lp);
        CPLEX_edit_post_heur(&CPLEX_env, &CPLEX_lp, inst->solution, inst->nnodes);


        remove_fix(&CPLEX_env, &CPLEX_lp, x, x_size);
    }

    double end_time = get_time();
    #if VERBOSE > 0
		print_lifespan(end_time,start_time);
	#endif
}