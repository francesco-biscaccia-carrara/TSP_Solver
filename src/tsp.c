#include "../include/tsp.h"

#define SQUARE(x)   (x*x)

#pragma region static_functions

static double euc_2d(point* a, point* b) {
    double dx = b->x - a->x;
    double dy = b->y - a->y; 

    return ((int) (sqrt(SQUARE(dx) + SQUARE(dy)) + 0.5)) + 0.0;
}

static void tsp_read_file(instance * problem, const char* file){
    FILE *f = fopen(file, "r");

    if ( f == NULL ){
        instance_delete(problem);
        print_error("input file not found!");
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
                instance_delete(problem);
                print_error(" format error: only TYPE: TSP implemented!"); 
            }
        }
        
        if (!strncmp(par_name, "DIMENSION", 9)){
            if (problem->nnodes > 0){
                instance_delete(problem);
                print_error(" repeated DIMENSION section in input file!");
            }
            
            problem->nnodes = atoi(strtok(NULL, " :"));
            problem->points = (point *) calloc(problem->nnodes, sizeof(point));
            problem->edge_weights = (double *) calloc(((problem->nnodes*(problem->nnodes-1)/2)), sizeof(double));
            
            if (problem->points == NULL) print_error(" failed to allocate memory for points vector!");
            if (problem->edge_weights == NULL) print_error(" failed to allocate memory for edge_weights vector!");

        }

        if (!strncmp(par_name, "EDGE_WEIGHT_TYPE", 16)){
            if (strncmp(strtok(NULL, " :"), "ATT",3)){
                instance_delete(problem);
                print_error(" format error: only EDGE_WEIGHT_TYPE: ATT implemented!"); 
            }
        }

        if (!strncmp(par_name, "NODE_COORD_SECTION", 18)) node_section=1;

        if (!strncmp(par_name, "EOF", 3)) node_section=0;

        if(node_section){
            int i = atoi(par_name)-1;
            if ( i >= 0 && i < problem->nnodes ){
                problem->points[i].x = atof(strtok(NULL, " "));
                problem->points[i].y = atof(strtok(NULL, " "));
            }
        }

    }
}

static void tsp_generate_random_point(uint32_t nnodes, uint32_t seed, instance* inst) {
    
    inst->nnodes = nnodes;
    inst->random_seed=seed;
    inst->points = (point *) calloc(inst->nnodes, sizeof(point));
    inst->edge_weights = (double *) calloc(((inst->nnodes*(inst->nnodes-1)/2)), sizeof(double));
    
    srand(seed);

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

#pragma endregion

instance* instance_new() {
    instance *problem = (instance*) calloc(1,sizeof(instance));

    problem->nnodes = 0;
    problem->random_seed = 0;
    problem->points = NULL;
    problem->edge_weights = NULL;
    problem->solution = NULL;
    problem->cost = DBL_MAX;

    #if VERBOSE > 1
    printf("\e[1mGENERATE NEW ISTANCE\e[m\n");
    #endif

    return problem;
}

instance* instance_new_cli(cli_info* cli_info) {
    instance *problem = instance_new();

    if(!strncmp(cli_info->file_name,"RND",3) || fopen(cli_info->file_name, "r")==NULL){
        tsp_generate_random_point(cli_info->nnodes,cli_info->random_seed,problem);
    }else{
        tsp_read_file(problem,cli_info->file_name);
    }

    #if VERBOSE > 0
        printf("------\e[1mInstance data\e[m------");
        printf("\n - nodes : %ld",problem->nnodes);
        if(problem->random_seed) printf("\n - random_seed : %u",problem->random_seed);
        printf("\n-------------------------\n\n");
    #endif 

    return problem;
}

void instance_delete(instance* problem) {

    free(problem->points);
    free(problem->edge_weights);
    free(problem->solution);
    free(problem);

    #if VERBOSE > 1
    printf("\e[1mDELETE THE ISTANCE\e[m\n");
    #endif
}

double tsp_save_weight(instance * problem, int i, int j){
    if (i == j) return 0;
    int ind = coords_to_index(problem->nnodes,i,j);

    if(!problem->edge_weights[ind])
        problem->edge_weights[ind] = euc_2d(&(problem->points[i]), &(problem->points[j]));
      
    return problem->edge_weights[ind];
}