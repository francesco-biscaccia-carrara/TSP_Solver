#include "include/tsp_solver.h"
#include "include/tsp_exact.h"
#include "include/matheuristic.h"

#define SET_SIZE     2
int test(int argc, char** argv) {
    
    char* methods[SET_SIZE] = {"GREEDY", "G2OPT"};
    double solutions[SET_SIZE + 1];
    TSPenv* env = environment_new_cli(argv, argc);
    
    FILE* f = fopen("data.csv", "w");
    fprintf(f,"%i", SET_SIZE);
    for(int k = 0; k < SET_SIZE; k++) { fprintf(f,",%s", methods[k]); }
    fprintf(f,"\n");

    for(int i = 0; i < 100; i++) {
        solutions[0] = i;
        for(int j = 0; j < SET_SIZE; j++) {
            environment_set_method(env, methods[j]);
            environment_set_seed(env, i);
            TSPinst* inst = instance_new_env(env);
            TSPsolve(inst, env);
            solutions[j+1] = inst->cost;
            free(inst);
        }
        format_csv_line(f, solutions, SET_SIZE+1);
    }
}

int main(int argc, char **argv) {
    TSPenv* env = environment_new_cli(argv, argc);
    TSPinst* inst = instance_new_env(env);

    char* cplex_func[] = { "BENDER", "PATCHING","BRANCH_CUT"};
    
    if(!strncmp(env->method,"DIVING", 6)) diving(inst, env);
    else if(strnin(env->method, cplex_func, 3)) {
        TSPCsolve(inst,env);
        TSPg2optb(inst, inst->solution, &(inst->cost));
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