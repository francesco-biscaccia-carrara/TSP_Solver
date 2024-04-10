#include "include/tsp_solver.h"
#include "include/tsp_exact.h"

int main(int argc, char **argv) {
    TSPenv* env = environment_new_cli(argv, argc);
    printf("%s\n", env->file_name);
    TSPinst* inst = instance_new_env(env);

    if(!strcmp(env->method,"CPLEX")) tsp_bender_loop(inst,env);
	else TSPsolve(inst, env);

    instance_delete(inst);
    environment_delete(env);
    return 0;
}