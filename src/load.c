#include "../include/load.h"

#define MAX_TIME    3.6e+6	//An hour

#pragma region static_functions

static void help_info(){
    printf("\e[1mTo set the parameters properly you have to execute tsp and add:\e[m");
    printf("\n '-in / -f / -file <filename.tsp>' to specity the input file; ");
    printf("\n '-tl / -max_time <time_dbl>' to specity the max execution time (int value);");
    printf("\n '-n / -n_nodes <num_nodes_int>' to specify the number of nodes in the TSP instance (int value);");
    printf("\n '-algo / -method / -alg <method>' to specify the method to solve the TSP instance;");
    printf("\n\tImplemented method: \n\t\t- GREEDY = greedy search,\n\t\t- G2OPT_F = greedy + 2opt w. first swaps,\n\t\t- G2OPT_B = greedy + 2opt w. best swaps");
    printf("\n '-seed / -rnd_seed <seed>' to specity the random seed (int value);");
    printf("\n '-multi_th / -mt' to use multithreading computation;");
    printf("\n '-help / --help / -h' to get help.");
    printf("\n\nNOTICE: you can insert only .tsp file or random seed and number of nodes, NOT BOTH!\n");
}

#pragma endregion

void parse_cli(int argc, char **argv,cli_info* cli_data){
    cli_data->nnodes=0;
    cli_data->random_seed=0;
    cli_data->time_limit = MAX_TIME;
    cli_data->mt = 0;
    strcpy(cli_data->method,"");
    strcpy(cli_data->file_name,"");

    for (int i = 1; i < argc; i++) { 
        if (!strcmp(argv[i],"-tl") ||!strcmp(argv[i],"-max_time")) cli_data->time_limit = abs(atoi(argv[++i])) + 0.0;
        if (!strcmp(argv[i],"-in")  || !strcmp(argv[i],"-f")  || !strcmp(argv[i],"-file"))  strcpy(cli_data->file_name,argv[++i]);
        if (!strcmp(argv[i],"-n") || !strcmp(argv[i],"-n_nodes")) cli_data->nnodes = abs(atoi(argv[++i]));
        if (!strcmp(argv[i],"-algo") || !strcmp(argv[i],"-method") || !strcmp(argv[i],"-alg")) strcpy(cli_data->method,argv[++i]);
        if (!strcmp(argv[i],"-seed") || !strcmp(argv[i],"-rnd_seed") || !strcmp(argv[i],"-s")) cli_data->random_seed = abs(atoi(argv[++i])); 
        if (!strcmp(argv[i],"-mt") ||!strcmp(argv[i],"-multi_th")) cli_data->mt = 1;  
        if (!strcmp(argv[i],"-help")  || !strcmp(argv[i],"--help") || !strcmp(argv[i],"-h"))   {help_info(); exit(0);}  
    } 

    if(cli_data->nnodes && cli_data->random_seed) strcpy(cli_data->file_name,"RND");
}