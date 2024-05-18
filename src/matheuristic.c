#include "../include/matheuristic.h"


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

void remove_fix(CPXENVptr env, CPXLPptr lp, int* variable_to_fix, int variable_to_fix_size) {
    char L = 'L';
    double v = 0.0;
    for(int i = 0; i < variable_to_fix_size; i++) {
        CPXchgbds(env, lp, 1, variable_to_fix+i, &L, &v);
    }
}


void diving(TSPinst* inst, TSPenv* env) {

    CPXENVptr CPLEX_env = NULL; 
	CPXLPptr CPLEX_lp = NULL;
	CPLEX_model_new(inst, &CPLEX_env, &CPLEX_lp);
    
    TSPgreedy(inst, rand()%inst->nnodes, NULL, "");
    CPLEX_post_heur(&CPLEX_env, &CPLEX_lp, inst->solution, inst->nnodes);
    int* x = calloc(inst->nnodes, sizeof(int));
    int x_size = 0;
    double start_time = get_time();
    while(time_elapsed(start_time) < env->time_limit) {
        x_size = fix_arc(x, inst->solution, 1, inst->nnodes);
        fix_to_model(&CPLEX_env, &CPLEX_lp, x, x_size);

        TSPCbranchcut(inst, env, &CPLEX_env, &CPLEX_lp);
        CPLEX_post_heur(&CPLEX_env,&CPLEX_lp,inst->solution,inst->nnodes);

        remove_fix(CPLEX_env, CPLEX_lp, x, x_size);
    }

    CPLEX_model_delete(&CPLEX_env,&CPLEX_lp);
}