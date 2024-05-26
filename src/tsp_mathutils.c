#include "../include/tsp_mathutils.h"

#pragma static_functions

static int rand_strategy(int* dest, int* solution, int p, int dest_size, int nnodes) {
    int k = 0;
    for(int i = 0; i < nnodes && k < dest_size; i++) {
        
        if(rand()%10 >= p) continue;
        if(i == nnodes-1)
            dest[k++] = coords_to_index(nnodes, solution[nnodes-1], solution[0]);
        
        else 
            dest[k++] = coords_to_index(nnodes, solution[i], solution[i+1]);
    
    }
    return k;
}

static int wght_strategy(int* dest, TSPinst* inst, int avg_cost, int dest_size, int nnodes) {
    int k = 0;
    for(int i = 0; i < nnodes-1 && k < dest_size; i++) {

        double arc_cost = get_arc(inst, inst->solution[i], inst->solution[i + 1]);
        if(arc_cost > avg_cost) {
            dest[k++] = coords_to_index(nnodes, inst->solution[i], inst->solution[i+1]);
        }
    }
    if(get_arc(inst, inst->solution[nnodes-1], inst->solution[0]) > avg_cost && k < dest_size)
        dest[k++] = coords_to_index(nnodes, inst->solution[nnodes-1], inst->solution[0]);

    return k;
}

static int prob_strategy() {
    return -1;
}

#pragma endregion


int arc_to_fix(int stategy, int* dest, TSPinst* inst, int p, int dest_size) {

    switch (stategy) {
    case Random:   
        return rand_strategy(dest, inst->solution, p, dest_size, inst->nnodes);
    case Weighted: 
        int r = rand()%10;
        return r < 2 ? rand_strategy(dest, inst->solution, p, dest_size, inst->nnodes) : wght_strategy(dest, inst, inst->cost/inst->nnodes, dest_size, inst->nnodes);
    //case Probably: return prob_strategy();
    default: 
        print_state (Error, "Strategy not found\n");
    }

}


void fix_to_model(CPXENVptr env, CPXLPptr lp, int* arcs_to_fix, int narcs) {

    char* ls = malloc(narcs);
    memset(ls, 'L', narcs);

    double* vs = malloc( narcs * sizeof(double) );
    for(int i = 0; i < narcs; i++) vs[i] = 1.0;

    CPXchgbds(env, lp, narcs, arcs_to_fix, ls, vs);
}


void unfix_to_model(CPXENVptr env, CPXLPptr lp, int* arcs_to_fix, int narcs) {
    char* ls = malloc(narcs);
    memset(ls, 'L', narcs);

    double* vs = calloc( narcs , sizeof(double) );

    CPXchgbds(env, lp, narcs, arcs_to_fix, ls, vs);
}


void local_tour_costraint(CPXENVptr env, CPXLPptr lp, TSPinst* inst, int k) {

    int* limit = calloc(inst->nnodes, sizeof( int ));
    double* value = calloc(inst->nnodes, sizeof( double ));

    for(int i = 0; i < inst->nnodes-1; i++) {
        limit[i] = coords_to_index(inst->nnodes, inst->solution[i], inst->solution[i+1]);
        value[i] = 1.0;
    } 
    limit[inst->nnodes-1] = coords_to_index(inst->nnodes, inst->solution[inst->nnodes - 1], inst->solution[0]);
    value[inst->nnodes-1] = 1.0;

    double rhs = inst->nnodes - k;
    char sense = 'G';
    int start_index = 0;
    
    if ( CPXaddrows(env,lp, 0,1, inst->nnodes,&rhs,&sense,&start_index,limit,value,NULL,NULL) ) 
        print_state(Error, "CPXaddrows() error");
}