#include "utils.h"

void tsp_free_instance(instance* inst){
    if(VERBOSE >= 10) printf("__log: deallocating instance's inputs\n");
    free(inst->points);
    free(inst->edge_weights);
}

void tsp_initialize_instance(instance * inst){
    inst->nnodes=0;
    inst->random_seed=0;
    inst->points=NULL;
    inst->edge_weights=NULL;
    inst->time_limit = MAX_TIME;
    inst->best_cost = DBL_MAX;
    inst->best_time = MAX_TIME;
    inst->best_sol = NULL;
    strcpy(inst->file_name,"NONE");
}

void print_error(const char *err){
    printf("\n\n__ERROR: %s\n\n", err); 
    fflush(NULL); 
    exit(1);
} 

uint64_t get_time(){
    return (uint64_t) time(NULL);
}

void tsp_read_file(instance * inst){
    FILE *f = fopen(inst->file_name, "r");
    if ( f == NULL ){
        tsp_free_instance(inst);
        print_error(" input file not found!");
    }
 
    inst->nnodes = 0;

    char line[251];
    char node_section = 0;
    char *par_name;   

    while(fgets(line, sizeof(line), f) != NULL){
        if (VERBOSE >= 10) { printf("__log: line: %s",line); fflush(NULL); }
        if (strlen(line) <= 1) continue; 

        if(!node_section) par_name = strtok(line, " :");
        else par_name=strtok(line, " ");

        if ( VERBOSE >= 10 ) {printf("__log: parameter \"%s\" \n",par_name); fflush(NULL); }

        if (!strncmp(par_name, "TYPE", 4)) {
            if (strncmp(strtok(NULL, " :"), "TSP",3)){
                tsp_free_instance(inst);
                print_error(" format error:  only TYPE: TSP implemented!"); 
            }
        }
        
        if (!strncmp(par_name, "DIMENSION", 9)){
            if (inst->nnodes > 0){
                tsp_free_instance(inst);
                print_error(" repeated DIMENSION section in input file!");
            }
            inst->nnodes = atoi(strtok(NULL, " :"));
            inst->points = (point *) calloc(inst->nnodes, sizeof(point));
            if (inst->points == NULL) print_error(" failed to allocate memory for points vector!");
            if (VERBOSE>=10) printf("__log: nnodes %d\n", inst->nnodes); 
        }

        if (!strncmp(par_name, "EDGE_WEIGHT_TYPE", 16)){
            if (strncmp(strtok(NULL, " :"), "ATT",3)){
                tsp_free_instance(inst);
                print_error(" format error: only EDGE_WEIGHT_TYPE: ATT implemented!"); 
            }
        }

        if (!strncmp(par_name, "NODE_COORD_SECTION", 18)) node_section=1;

        if (!strncmp(par_name, "EOF", 3)) node_section=0;

        if(node_section){
            int i = atoi(par_name)-1;
            if ( i >= 0 && i < inst->nnodes ){
                inst->points[i].x = atof(strtok(NULL, " "));
                inst->points[i].y = atof(strtok(NULL, " "));
                if (VERBOSE>=10) printf("__log: node %d at coordinates (%15.7lf, %15.7lf)\n", i+1, inst->points[i].x,inst->points[i].y);
            }
        }

    }
}

void tsp_compute_weights(instance * inst){
    uint32_t n = inst->nnodes;
    inst->edge_weights = (double *) calloc((n*n)/2-n/2, sizeof(double));
    
    if (inst->edge_weights == NULL) print_error(" failed to allocate memory for edge_weights vector!");

    int index = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++){
            inst->edge_weights[index++] = euc_2d(&(inst->points[i]), &(inst->points[j]));
            if (index<400) printf("__log: edge (%d,%d) -> %lf\n",i,j,inst->edge_weights[index-1]);
        }
    }
}

int tsp_convert_coord_edge(uint32_t n,int i,int j){
    return i<j ? (n * i - i * (i + 1) / 2) + (j - i - 1) : (n * i - i * (i + 1) / 2) + (j - i - 1);
}


void tsp_parse_cli(int argc, char** argv, instance *inst){
    if ( VERBOSE >= 10 ) printf("__log: CLI has %d pars \n", argc-1);
    tsp_initialize_instance(inst);

    char help = 0;

    if (argc < 1) help = 1;
    for (int i = 1; i < argc; i++) { 

		if (!strcmp(argv[i],"-in") || !strcmp(argv[i],"-f") || !strcmp(argv[i],"-file")) strcpy(inst->file_name,argv[++i]);

		if (!strcmp(argv[i],"-tl") ||!strcmp(argv[i],"-max_time")) inst->time_limit = atof(argv[++i]);

        if (!strcmp(argv[i],"-n") || !strcmp(argv[i],"-n_nodes")) inst->nnodes = abs(atoi(argv[++i]));
        
		if (!strcmp(argv[i],"-seed") || !strcmp(argv[i],"-rnd_seed")) inst->random_seed = abs(atoi(argv[++i]));			

		if (!strcmp(argv[i],"-help") || !strcmp(argv[i],"--help") || !strcmp(argv[i],"-h"))   help = 1;				
    }     

    if(inst->nnodes && inst->random_seed) strcpy(inst->file_name,"NONE");

    if(help){
            printf("To set the parameters properly you have to execute tsp and add:");
            printf("\n '-in / -f / -file <filename.tsp>' to specity the input file; ");
            printf("\n '-tl / -max_time <time_dbl>' to specity the max execution time (double value);");
            printf("\n '-n / -n_nodes <num_nodes_int>' to specity the number of nodes in the TSP instance (int value);");
            printf("\n '-seed / -rnd_seed <seed>' to specity the random seed (int value);");
            printf("\n '-help / --help / -h' to get help.");
            printf("\n\nNOTICE: you can insert only .tsp file or random seed and number of nodes, NOT BOTH!\n");
    }
   
    if(VERBOSE >=5){
        printf("------Instance data------");
        printf("\n - nodes : %d",inst->nnodes);
        printf("\n - random_seed : %u",inst->random_seed);
        printf("\n - time_limit : %lu",inst->time_limit);
        printf("\n - file_name : %s",inst->file_name);
        printf("\n-------------------------\n");
    }    
}

void tsp_create_plot_data(instance *inst){
    FILE* file = fopen(".solution.dat","w");
    int sol[]={0,2,1,4,5,0}; //--> inst->best_sol
    for(int i=0;i < (sizeof(sol)/sizeof(int))-1; i++){ //(sizeof(sol)/sizeof(int))-1 --> inst->nnodes -1
        fprintf(file,"%lf %lf\n",inst->points[sol[i]].x,inst->points[sol[i]].y);
        fprintf(file,"%lf %lf\n",inst->points[sol[i+1]].x,inst->points[sol[i+1]].y);
        fprintf(file,"\n");
    }
    fclose(file);
}

void tsp_plot(instance *inst){
    tsp_create_plot_data(inst);
    FILE* gnuplot_pipe = popen("gnuplot -persistent","w");
    const char *COMMANDS[] ={
        "set title 'TSP Solution'",
        "set xrange [0:10000]",
        "set yrange [0:10000]",
        "unset key",
        "plot '.solution.dat' with linespoints linetype 7 linecolor 6",
        0};

    for (int i=1;COMMANDS[i];i++){
       fprintf(gnuplot_pipe,"%s \n",COMMANDS[i]); 
    }
    pclose(gnuplot_pipe);
}

