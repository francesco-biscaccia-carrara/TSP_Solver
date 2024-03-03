#include "utils.h"

const char *WEIGHT_TYPE[] = {"12","EXPLICIT", "EUC_2D", "EUC_3D", "MAX_2D", "MAX_3D", "MAN_2D", "MAN_3D","CEIL_2D", "GEO", "ATT", "XRAY1", "XRAY2"};

void free_instance(instance* inst){
    if(VERBOSE >= 10) printf("__log: deallocating instance's inputs\n");
    free(inst->points);
}

void print_error(const char *err){
    printf("\n\n__ERROR: %s\n\n", err); 
    fflush(NULL); 
    exit(1);
} 

uint64_t get_time(){
    return (uint64_t) time(NULL);
}

void read_tsp_file(instance * inst){
    FILE *f = fopen(inst->file_name, "r");
    if ( f == NULL ){
        free_instance(inst);
        print_error(" input file not found!");
    }
 
    inst->nnodes = 0;

    char line[251];
    char line_cp[251];
    char node_section = 0;
    char *par_name;   
    char *token;

    while(fgets(line, sizeof(line), f) != NULL){
        if (VERBOSE >= 10) { printf("__log: line: %s",line); fflush(NULL); }
        if (strlen(line) <= 1) continue; 
        strcpy(line_cp, line);
        par_name = strtok(line, " :");
        if ( VERBOSE >= 10 ) {printf("__log: parameter \"%s\" \n",par_name); fflush(NULL); }

        if (!strncmp(par_name, "TYPE", 4)) {
            if (strncmp(strtok(NULL, " :"), "TSP",3)){
                free_instance(inst);
                print_error(" format error:  only TYPE == TSP implemented!"); 
            }
        }
        
        if (!strncmp(par_name, "DIMENSION", 9)){
            if (inst->nnodes > 0){
                free_instance(inst);
                print_error(" repeated DIMENSION section in input file!");
            }
            inst->nnodes = atoi(strtok(NULL, " :"));
            if (VERBOSE>=10) printf("__log: nnodes %d\n", inst->nnodes); 
            inst->points = (point *) calloc(inst->nnodes, sizeof(point));
        }

        if (!strncmp(par_name, "EDGE_WEIGHT_TYPE", 16)){
            int i;
            token = strtok(NULL, " :");
            token[strlen(token)-1]=0;
            for(i=1;i <= atoi(WEIGHT_TYPE[0]);i++){
                if(!strncmp(token, WEIGHT_TYPE[i],strlen(token))){
                    strcpy(inst->weight_type,WEIGHT_TYPE[i]);
                    break;
                }
                
            }
            if(i == atoi(WEIGHT_TYPE[0])){
                free_instance(inst);
                print_error(" format error:  this format is not valid for TSP problem!");
            }
            if (VERBOSE>=10) printf("__log: weight type %s\n", inst->weight_type); 
        }

        if (!strncmp(par_name, "NODE_COORD_SECTION", 18)) node_section=1;

        if (!strncmp(par_name, "EOF", 3)) node_section=0;

        if(node_section){
            int i = atoi(strtok(line_cp, " "))-1;
            if ( i < 0 || i >= inst->nnodes ) continue;
            token = strtok(NULL, " ");
            inst->points[i].x = atof(token);
            token = strtok(NULL, " ");
            inst->points[i].y = atof(token);
            if (VERBOSE>=10) printf("__log: node %d at coordinates (%15.7lf, %15.7lf)\n", i+1, inst->points[i].x,inst->points[i].y);
        }

    }
}

void parse_cli(int argc, char** argv, instance *inst){
    if ( VERBOSE >= 10 ) printf("__log: CLI has %d pars \n", argc-1);

    //Default init of an instance
    inst->nnodes=0;
    inst->random_seed=0;
    inst->points=NULL;
    inst->time_limit = MAX_TIME;
    inst->best_cost = DBL_MAX;
    inst->best_time = MAX_TIME;
    inst->best_sol = NULL;
    strcpy(inst->file_name,"NONE");
    strcpy(inst->weight_type,"NONE");

    int help = 0;

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

void solution_file(instance *inst){
    FILE* file = fopen(".solution.dat","w");
    int sol[]={0,2,1,4,5,0}; //--> inst->best_sol
    for(int i=0;i < (sizeof(sol)/sizeof(int))-1;i++){ //(sizeof(sol)/sizeof(int))-1 --> inst->nnodes -1
        fprintf(file,"%lf %lf\n",inst->points[sol[i]].x,inst->points[sol[i]].y);
        fprintf(file,"%lf %lf\n",inst->points[sol[i+1]].x,inst->points[sol[i+1]].y);
        fprintf(file,"\n");
    }
    fclose(file);
}

void plot(instance *inst){
    solution_file(inst);
    FILE* gnuplot_pipe = popen("gnuplot -persistent","w");
    const char *COMMANDS[] ={
        "5",
        "set title 'TSP Solution'",
        "set xrange [0:10000]",
        "set yrange [0:10000]",
        "unset key",
        "plot '.solution.dat' with linespoints linetype 7 linecolor 6",
    };

    for (int i=1;i<=atoi(COMMANDS[0]);i++){
       fprintf(gnuplot_pipe,"%s \n",COMMANDS[i]); 
    }
    pclose(gnuplot_pipe);
}

