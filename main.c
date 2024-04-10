#include "include/tsp_solver.h"
#include "include/tsp_exact.h"

int main(int argc, char **argv) {
    TSPenv* env = environment_new_cli(argv, argc);
    printf("%s\n", env->file_name);
    TSPinst* inst = instance_new_env(env);

    if(!strcmp(env->method,"CPLEX")) tsp_bender_loop(inst,env);
	else TSPsolve(inst, env);

    FILE* f = fopen("test.txt", "w");
    plot_log(inst, f);
    fclose(f);

    instance_delete(inst);
    environment_delete(env);
    system("cd plot && python3.10 plot_solution.py");
    return 0;
}