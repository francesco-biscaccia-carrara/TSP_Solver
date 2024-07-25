#include "../include/tsp.h"

double* edge_weights;

#pragma region static_functions

/// @brief print output for help function
static void help_info(){
    printf("\e[1mTo set the parameters properly you have to execute tsp and add:\e[m");
    printf("\n '-in / -f / -file <filename.tsp>' to specity the input file; ");
    printf("\n '-tl / -max_time <time_dbl>' to specity the max execution time (int value);");
    printf("\n '-n / -n_nodes <num_nodes_int>' to specify the number of nodes in the TSP instance (int value);");
    printf("\n '-seed / -rnd_seed <seed>' to specity the random seed (int value);");
    printf("\n '-algo / -method / -alg <method>' to specify the method to solve the TSP instance;");
    printf("\n Implemented method:\
    \n\t- GREEDY = greedy search\
    \n\t- G2OPT_F = greedy + 2opt w. first swaps\
    \n\t- G2OPT_B = greedy + 2opt w. best swaps\
    \n\t- TABU_R = tabu search w. greedy as starting solution\
    \n\t- TABU_B = tabu search w. greedy + 2opt as starting solution\
    \n\t- VNS = vns search w. 2opt best swaps\
    \n\t- BENDERS = benders' loop\
    \n\t- BRANCH_CUT = branch-and-cut\
    \n\t- DIVING_R = diving w. random fixed edges\
    \n\t- DIVING_W = diving w. weighted fixed edges\
    \n\t- LOCAL_BRANCH = diving w. random fixed edges\
    ");
    printf("\n '-help / --help / -h' to get help.");
    printf("\n\n\e[1m\e[4mNOTICE\e[0m: you can insert only .tsp file or random seed and number of nodes, \e[4mNOT BOTH\e[0m!\n");
}


/// @brief function to provide a random instance to test functionality of TSP solver
/// @param inst instance of TSPinst
/// @param nnodes number of nodes inside TSP inst
/// @param seed seed to set random instance.
static void tsp_rnd_inst(TSPinst* inst, unsigned int nnodes, const unsigned int seed) {
    inst->nnodes = nnodes;
    inst->random_seed = seed;
    inst->points = (point *) calloc(inst->nnodes, sizeof(point));
    edge_weights = (double *) calloc(((inst->nnodes*(inst->nnodes-1)/2)), sizeof(double));
    inst->solution = malloc(nnodes * sizeof(int));

    srand(seed);

    if(nnodes <= 1) { print_state(Error, "Impossible to generate problem with less than 2 nodes\n"); } 

    #if VERBOSE > 0
        printf("\e[1mGENERATE RANDOM POINT...\e[m\n");
    #endif

    for(int i = 0; i < nnodes; i++) {
        point p = {.x = rand() % MAX_DIST, .y = rand() % MAX_DIST};
        inst->points[i] = p;
        
        #if VERBOSE > 1 
        printf("x_%i = (%10.4f, %10.4f) \n",i, p.x, p.y); 
        #endif
    }
    #if VERBOSE > 0 
        printf("\n"); 
    #endif
}


/// @brief parser of .tsp file
/// @param inst instance of TSPinst
/// @param file name of .tsp file
static void tsp_read_file(TSPinst* inst, const char* file) {
    FILE *f = fopen(file, "r");

    if ( f == NULL ){
        instance_delete(inst);
        print_state(Error, "input file not found!");
    }

    char line[201];
    char node_section = 0;
    char *par_name;   

    while(fgets(line, sizeof(line), f) != NULL){
    
        if (strlen(line) <= 1) continue; 

        if(!node_section) par_name = strtok(line, " :");
        else par_name=strtok(line, " ");

        if (!strncmp(par_name, "TYPE", 4)) {
            if (strncmp(strtok(NULL, " :"), "TSP",3)){
                instance_delete(inst);
                print_state(Error, " format error: only TYPE: TSP implemented!"); 
            }
        }
        
        if (!strncmp(par_name, "DIMENSION", 9)){
            if (inst->nnodes > 0){
                instance_delete(inst);
                print_state(Error, " repeated DIMENSION section in input file!");
            }
            
            inst->nnodes = atoi(strtok(NULL, " :"));
            inst->points = (point *) calloc(inst->nnodes, sizeof(point));
            edge_weights = (double *) calloc(((inst->nnodes*(inst->nnodes-1)/2)), sizeof(double));
            inst->solution = malloc(inst->nnodes * sizeof(int));

            if (inst->points == NULL) print_state(Error, " failed to allocate memory for points vector!");
            if (edge_weights == NULL) print_state(Error, " failed to allocate memory for edge_weights vector!");

        }

        if (!strncmp(par_name, "EDGE_WEIGHT_TYPE", 16)){
            if (strncmp(strtok(NULL, " :"), "ATT",3)){
                instance_delete(inst);
                print_state(Error, " format error: only EDGE_WEIGHT_TYPE: ATT implemented!"); 
            }
        }

        if (!strncmp(par_name, "NODE_COORD_SECTION", 18)) node_section=1;

        if (!strncmp(par_name, "EOF", 3)) node_section=0;

        if(node_section){
            int i = atoi(par_name)-1;
            if ( i >= 0 && i < inst->nnodes ){
                inst->points[i].x = atof(strtok(NULL, " "));
                inst->points[i].y = atof(strtok(NULL, " "));
            }
        }

    }
}

#pragma endregion


/// @brief default constructor of TSPinst
/// @return an instance of TSPinst
TSPinst* instance_new() {
    TSPinst *inst = (TSPinst*) calloc(1,sizeof(TSPinst));
    inst->cost = DBL_MAX;

    #if VERBOSE > 1
    printf("\e[1mGENERATE NEW ISTANCE\e[m\n");
    #endif

    return inst;
}


/// @brief build TSPinst with TSPenv data
/// @param env instance of TSPenv
/// @return an instance of TSPinst
TSPinst* instance_new_env(TSPenv* env) {
    TSPinst* inst = instance_new();

    init_random();

    if(!strncmp(env->file_name,"RND",3) || fopen(env->file_name, "r") == NULL ){
        tsp_rnd_inst(inst, env->nnodes,env->random_seed);
    }else{
        tsp_read_file(inst,env->file_name);
    }

    #if VERBOSE > 0
        printf("------\e[1mInstance data\e[m------");
        printf("\n - nodes : %i",inst->nnodes);
        if(inst->random_seed) printf("\n - random_seed : %u",inst->random_seed);
        printf("\n-------------------------\n\n");
    #endif

    return inst;
}


/// @brief free memory of an instance of TSPinst
/// @param inst instance of TSPinst
void instance_delete(TSPinst* inst) {
    free(inst->points);
    free(edge_weights);
    free(inst->solution);
    free(inst);

    #if VERBOSE > 1
    printf("\e[1mDELETE THE ISTANCE\e[m\n");
    #endif
}


/// @brief setter for a "solution" (tour and cost) inside TSPinst
/// @param inst instance of TSPinst
/// @param tour hamiltonian circuit
/// @param cost cost of tour
void instance_set_solution(TSPinst* inst, const int* tour, const double cost) {
    inst->cost = cost;
    memcpy(inst->solution, tour, inst->nnodes * sizeof(inst->solution[0]));
}

void instance_set_best_sol(TSPinst* inst, const TSPsol sol) {
    if(sol.cost < inst->cost){
        instance_set_solution(inst,sol.tour,sol.cost);
    }
}


/*===============================================================================*/


/// @brief default constructor of TSPenv
/// @return an instance of TPSenv
TSPenv* environment_new() {
    TSPenv *environment = (TSPenv*) calloc(1,sizeof(TSPenv));
    environment->file_name = calloc(64, sizeof(char));
    environment->method = calloc(23, sizeof(char));
    environment->time_limit = MAX_TIME;
    environment->time_exec = 0;
    environment->perf_v = 0;

    #if VERBOSE > 1
    printf("\e[1mGENERATE NEW ENVIRONMENT\e[m\n");
    #endif

    return environment;
}


/// @brief build TSPenv with cli arguments
/// @param argv arguments value passed by cli 
/// @param argc arguments size passed by cli
/// @return an instance of TSPenv
TSPenv* environment_new_cli(char** argv, const int argc) {
    TSPenv* env = environment_new();

    char* time_comm[] = {"-tl", "-max_time"};
    char* file_comm[] = {"-in", "-f", "-file"};
    char* node_comm[] = {"-n", "-n_nodes"};
    char* algo_comm[] = {"-algo", "-method", "-alg"};
    char* seed_comm[] = {"-seed", "-rnd_seed", "-s"};
    char* help_comm[] = {"-help", "-h", "--help"};
//  char* warm_comm[] = {"-warm", "-w", "--warm"};
    char* perf_comm[] = {"-test", "-t"};
//  char* tabu_comm[] = {"-tabu_par", "-tp"};
//  char* vns_comm[] = {"-vns_par", "-vp"};

    for (int i = 1; i < argc; i++) { 
        if (strnin(argv[i], time_comm, 2))  env->time_limit = abs(atof(argv[++i]));
        if (strnin(argv[i], file_comm, 3))  strcpy(env->file_name,argv[++i]);
        if (strnin(argv[i], node_comm, 2))  env->nnodes = abs(atoi(argv[++i]));
        if (strnin(argv[i], algo_comm, 3))  strcpy(env->method,argv[++i]);
        if (strnin(argv[i], seed_comm, 3))  env->random_seed = abs(atoi(argv[++i])); 
//      if (strnin(argv[i], warm_comm, 2))  env->warm = 1;  
        if (strnin(argv[i], perf_comm, 2))  env->perf_v = 1;
//      if (strnin(argv[i], tabu_comm, 2))  env->tabu_par = abs(atoi(argv[++i]));  
//      if (strnin(argv[i], vns_comm, 2))   env->vns_par = abs(atoi(argv[++i]));
        if (strnin(argv[i], help_comm, 3))  { help_info(); exit(0); }  
    }

    if(env->nnodes && env->random_seed) strcpy(env->file_name,"RND");
    return env; 
}


/// @brief free memory of an instance of TSPenv
/// @param env instance of TSPenv
void environment_delete(TSPenv* env) {
    free(env->file_name);
    free(env->method);
    free(env);

    #if VERBOSE > 1
    printf("\e[1mDELETE THE ENVIRONMENT\e[m\n");
    #endif
}


/// @brief setter of method inside TSPenv
/// @param env instance of TSPenv
/// @param method_name name of desired method to solve TSP
void environment_set_method(TSPenv* env, char* method_name) {
    env->method = method_name;
}


/// @brief setter of seed inside TSPenv
/// @param env instance of TSPenv
/// @param seed number for random seed 
void environment_set_seed(TSPenv* env, const unsigned int seed) {
    env->random_seed = seed;
}