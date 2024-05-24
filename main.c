#include "include/tsp_solver.h"
#include "include/tsp_exact.h"
#include "include/matheuristic.h"


int main(int argc, char **argv) {
    TSPenv* env = environment_new_cli(argv, argc);
    TSPinst* inst = instance_new_env(env);

    char* cplex_func[] = { "BENDER", "PATCHING","BRANCH_CUT" };
    char* mathe_func[] = { "DIVING", "LOCAL BRANCH" };
    
    if(strnin(env->method, mathe_func, 2)) { 
        MATsolve(inst, env);
        TSPg2optb(inst, inst->solution, &(inst->cost));    
    }
    else if(strnin(env->method, cplex_func, 3)) {
        TSPCsolve(inst,env);
        check_tour_cost(inst, inst->solution, inst->cost);
    }
	else TSPsolve(inst, env);

    FILE* f = fopen("plot/input/test.txt", "w");
    plot_log(inst, f);
    fclose(f);

    print_sol(inst, env);
    instance_delete(inst);
    environment_delete(env);
    system("cd plot && python3 plot_solution.py &");
    return 0;
}