#include "include/tsp_solver.h"
#include "include/tsp_exact.h"

int test(int argc, char** argv) {
    char* methods[3] = {"GREEDY", "G2OPT_B", "G2OPT_F"};
    double solutions[4];
    TSPenv* env = environment_new_cli(argv, argc);
    
    FILE* f = fopen("data.csv", "w");
    fprintf(f,"%i", 3);
    for(int k = 0; k < 3; k++) { fprintf(f,",%s", methods[k]); }
    fprintf(f,"\n");

    for(int i = 0; i < 100; i++) {
        solutions[0] = i;
        for(int j = 0; j < 3; j++) {
            environment_set_method(env, methods[j]);
            environment_set_seed(env, i);
            TSPinst* inst = instance_new_env(env);
            TSPsolve(inst, env);
            solutions[j+1] = inst->cost;
            free(inst);
        }
        format_csv_line(f, solutions, 4);
    }
}

int main(int argc, char **argv) {
    TSPenv* env = environment_new_cli(argv, argc);
    printf("%s\n", env->file_name);
    TSPinst* inst = instance_new_env(env);
    
    if(!strcmp(env->method,"CPLEX")) {
        tsp_bender_loop(inst,env);
        TSPg2optb(inst, inst->solution, &(inst->cost));
        //check_tour_cost(inst, inst->solution, inst->cost);
    }
	else TSPsolve(inst, env);

    FILE* f = fopen("plot/input/test.txt", "w");
    plot_log(inst, f);
    fclose(f);

    instance_delete(inst);
    //environment_delete(env);
    system("cd plot && python3.10 plot_solution.py");
    return 0;
}