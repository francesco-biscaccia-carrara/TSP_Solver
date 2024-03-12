#include "../include/tsp.h"

instance* instance_new(){
    instance *problem = malloc(sizeof(instance));

    problem->nnodes = 0;
    problem->random_seed = 0;
    problem->points = NULL;
    problem->edge_weights = NULL;
    problem->combination = NULL;
    problem->result = UINT32_MAX;
    return problem;
}

void instance_delete(instance* problem){
    #if VERBOSE > 10 
    printf("__log: deallocating instance's inputs\n");
    #endif
 
    if(problem->points != NULL) free(problem->points);
    if(problem->edge_weights != NULL) free(problem->edge_weights);
    if(problem->combination!=NULL) free(problem->combination);
}

void tsp_generate_random_point(uint32_t nnodes, uint32_t seed, instance* inst) {
    inst->points = malloc(nnodes * sizeof(point));
    inst->nnodes = nnodes;
    inst->random_seed=seed;
    inst->points = (point *) calloc(inst->nnodes, sizeof(point));
    inst->edge_weights = (double *) calloc(((inst->nnodes*(inst->nnodes-1)/2)), sizeof(double));
    srand(seed);

    int i = 0; for(; i < nnodes; i++) {
        point p = {rand() % MAX_DIST, rand() % MAX_DIST};
        inst->points[i] = p;
        
    #if VERBOSE > 5 
    printf("(%10.4f, %10.4f) \n", p.x, p.y); 
    #endif
    }
}

void tsp_read_file(instance * problem, const char* file){
    FILE *f = fopen(file, "r");
    if ( f == NULL ){
        instance_delete(problem);
        print_error(" input file not found!");
    }

    char line[251];
    char node_section = 0;
    char *par_name;   

    while(fgets(line, sizeof(line), f) != NULL){
        #if VERBOSE > 9
        printf("__log: line: %s",line); 
        fflush(NULL); 
        #endif 
    
        if (strlen(line) <= 1) continue; 

        if(!node_section) par_name = strtok(line, " :");
        else par_name=strtok(line, " ");

        #if VERBOSE > 9 
        printf("__log: parameter \"%s\" \n",par_name); 
        fflush(NULL);
        #endif

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

            #if VERBOSE > 9
            printf("__log: nnodes %ld\n", problem->nnodes);
            #endif
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
                #if VERBOSE > 9 
                printf("__log: node %d at coordinates (%10.4f, %10.4f)\n", i+1, problem->points[i].x,problem->points[i].y);
                #endif
            }
        }

    }
}

void tsp_instance_from_cli(instance *problem, cli_info* cli){
    if(!strncmp(cli->file_name,"RND",3) || fopen(cli->file_name, "r")==NULL){
        tsp_generate_random_point(cli->nnodes,cli->random_seed,problem);
    }else{
        tsp_read_file(problem,cli->file_name);
    }

    #if VERBOSE > 5
        printf("------Instance data------");
        printf("\n - nodes : %ld",problem->nnodes);
        printf("\n - random_seed : %u",problem->random_seed);
        printf("\n-------------------------\n");
    #endif 
}

