#include "../include/tsp.h"
#include "../include/utils.h"
#include <math.h>

instance* instance_new(){
    instance *problem = malloc(sizeof(instance));

    problem->nnodes = 0;
    problem->random_seed = 0;
    problem->points = NULL;
    problem->edge_weights = NULL;
    problem->combination = NULL;
    problem->result = UINT32_MAX;
    
    #if VERBOSE > 5
    strcpy(problem->file_name,"NONE");
    #endif 

    return problem;
}

void instance_delete(instance* problem){
    #if VERBOSE > 10 
    printf("__log: deallocating instance's inputs\n");
    #endif
 
    if(problem->points != NULL) free(problem->points);
    if(problem->edge_weights != NULL) free(problem->edge_weights);
}

uint32_t tsp_save_weight(instance * problem, int i, int j){
    if (i == j) return 0;
    if(!problem->edge_weights[coords_to_index(problem->nnodes,i,j)])
        problem->edge_weights[coords_to_index(problem->nnodes,i,j)] = euc_2d(&(problem->points[i]), &(problem->points[j]));
      
    return problem->edge_weights[coords_to_index(problem->nnodes,i,j)];
}

point_n_dist get_min_distance_point(int index, instance *problem, uint32_t* res) {

    uint32_t min = UINT32_MAX, dist = 0;
    point_n_dist out;

    for(int i = 0; i < problem->nnodes; i++) {
        
        if(res[i] != -1) continue; //if not assigned
        dist = tsp_save_weight(problem,index,i);

        if (dist < min && dist != 0) {
            out.dist = dist;
            out.index = i;
            min = dist;
        }
    }

    return out;
}

void tsp_greedy(int index, instance* problem) {

    uint32_t cost = 0, current_index = index;
    point_n_dist new_point;

    //printf("BSOL IN GREEDY: %u\n", problem->result);

    //intialize solution
    int* result = malloc(sizeof(int)* problem->nnodes);
    for(int i = 0; i < problem->nnodes; i++) result[i] = -1;

    for (int i = 0; i < problem->nnodes; i++) {  
        new_point = get_min_distance_point(current_index, problem, result);

        cost += new_point.dist;
        result[current_index] = (current_index == new_point.index) ? index : new_point.index;
        current_index = new_point.index;
    }

    if(VERBOSE > 8) {
        printf("__log: BS Combination: ");
        for(int j = 0; j < problem->nnodes; j++) {
            printf("| %i", result[j]);
        }
        printf("|\n");
        printf("__log:BS Cost: %i\n", cost);
    }

    if(cost < problem->result){
        problem->result = cost;
        problem->combination = result;
    }
}

void tsp_generate_random_point(uint32_t nnodes, uint32_t seed, instance* inst) {
    inst->points = malloc(nnodes * sizeof(point));
    inst->nnodes = nnodes;
    inst->points = (point *) calloc(inst->nnodes, sizeof(point));
    inst->edge_weights = (int *) calloc(((inst->nnodes*(inst->nnodes-1)/2)), sizeof(int));
    srand(seed);

    int i = 0; for(; i < nnodes; i++) {
        point p = {rand() % MAX_DIST, rand() % MAX_DIST};
        inst->points[i] = p;
        
    #if VERBOSE > 5 
    printf("(%i, %i) \n", p.x, p.y); 
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
            problem->edge_weights = (int *) calloc(((problem->nnodes*(problem->nnodes-1)/2)), sizeof(int));
            
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
                printf("__log: node %d at coordinates (%d, %d)\n", i+1, problem->points[i].x,problem->points[i].y);
                #endif
            }
        }

    }
}

void tsp_instance_from_cli(int argc, char** argv, instance *problem){
    #if VERBOSE > 9
    printf("__log: CLI has %d pars \n", argc-1);
    #endif

    for (int i = 1; i < argc; i++) { 

		if (!strcmp(argv[i],"-in") || !strcmp(argv[i],"-f") || !strcmp(argv[i],"-file")){
            //Initialization of nnodes and random_seed
            problem->nnodes=0; 
            problem->random_seed=0;
            tsp_read_file(problem,argv[++i]);
            #if VERBOSE > 5
            strcpy(problem->file_name, argv[i]);
            #endif
            
        }

        if (!strcmp(argv[i],"-n") || !strcmp(argv[i],"-n_nodes") && !problem->nnodes) problem->nnodes = abs(atoi(argv[++i]));
        
		if (!strcmp(argv[i],"-seed") || !strcmp(argv[i],"-rnd_seed") && !problem->random_seed) problem->random_seed = abs(atoi(argv[++i]));			

		if (!strcmp(argv[i],"-help") || !strcmp(argv[i],"--help") || !strcmp(argv[i],"-h"))   {help_info(); exit(1);}				
    }     

    if(problem->nnodes && problem->random_seed) {
        #if VERBOSE >5 
        strcpy(problem->file_name,"RANDOM_INSTANCE");
        #endif
        tsp_generate_random_point(problem->nnodes,problem->random_seed,problem);
    }

    #if VERBOSE > 2
        printf("------Instance data------");
        printf("\n - nodes : %ld",problem->nnodes);
        printf("\n - random_seed : %u",problem->random_seed);
        printf("\n - file_name : %s",problem->file_name);
        printf("\n-------------------------\n");
    #endif 
}

void tsp_create_plot_data(instance *problem){
    FILE* file = fopen(".solution.dat","w");
    int j=0;
    for(int i=0;i < problem->nnodes; i++){ 
        fprintf(file,"%d %d\n",problem->points[j].x,problem->points[j].y);
        fprintf(file,"%d %d\n\n",problem->points[problem->combination[j]].x,problem->points[problem->combination[j]].y);
        j=problem->combination[j];
    }
    fclose(file);
}

void tsp_plot(instance *problem){
    tsp_create_plot_data(problem);
    FILE* gnuplot_pipe = popen("gnuplot -persistent","w");
    const char *COMMANDS[] ={
        "set title 'TSP Best Solution'",
        "set xrange [0:10000]",
        "set yrange [0:10000]",
        "unset key",
        "plot '.solution.dat' with linespoints linetype 7 linecolor 6",0};
    for (int i=1;COMMANDS[i];i++) fprintf(gnuplot_pipe,"%s \n",COMMANDS[i]); 
    pclose(gnuplot_pipe);
}

void print_best_solution_info(instance* problem){
    printf("Best solution: ");
	int j=0;
    for(int i=0;i < problem->nnodes; i++){ 
        printf("%d -> ",j);
        j=problem->combination[j];
    }
	printf("0\nBest solution cost: %d\n",problem->result);
}
