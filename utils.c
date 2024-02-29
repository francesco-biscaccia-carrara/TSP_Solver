#include "utils.h"

void free_instance(instance* inst){
    if(VERBOSE >= 10) printf("Deallocating instance's inputs\n");
    free(inst->points);
    free(inst);
}

void print_error(const char *err){
    printf("\n\nERROR: %s\n\n", err); 
    fflush(NULL); 
    exit(1);
} 

uint64_t get_time(){
    return (uint64_t) time(NULL);
}

void read_tsp_file(instance * inst){};

void parse_cli(int argc, char** argv, instance *inst){
    if ( VERBOSE >= 10 ) printf("CLI has %d pars \n", argc-1);

    //Default init of an instance
    inst->nnodes=0;
    inst->random_seed=0;
    inst->points=NULL;
    inst->time_limit = MAX_TIME;
    inst->best_cost = DBL_MAX;
    inst->best_time = MAX_TIME;
    inst->best_sol = NULL;
    strcpy(inst->file_name,"NONE");

    int help = 0;
    char no_input = 1;

    if (argc < 1) help = 1;
    for (int i = 1; i < argc; i++) { 

		if (!strcmp(argv[i],"-in") || !strcmp(argv[i],"-f") || !strcmp(argv[i],"-file"))  {
            strcpy(inst->file_name,argv[++i]);
            no_input=0;
        }

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
        printf("\n - time_limit : %llu",inst->time_limit);
        printf("\n - file_name : %s",inst->file_name);
        printf("\n-------------------------\n");
    }    
}

double euclidian_distance(point a, point b, short squared) {

    double dx = b.x - a.x;
    double dy = b.y - a.y; 

    if(squared) return sqrt((dx * dx) + (dy * dy));
    return ((dx * dx) + (dy * dy));
}