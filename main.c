#include "include/algorithm.h"
#include "include/display.h"
#include "include/load.h"
#include "include/tsp.h"

#define MAX_NODES_PLOT 300


int main(int argc, char **argv){
    if (argc < 2) {
		printf("Usage: %s -h/ -help/ --help/  to get some help\n", argv[0]);
		exit(1);
	} 

	cli_info cli_data;
	instance* problem=instance_new();
	char plot_name[100];

	parse_cli(argc,argv,&cli_data);
	strcpy(cli_data.method,"GREEDY");//HARDCODED CHANGE IT

	#if VERBOSE > 1
	printf("Time Limit\t: %lu ms\n",cli_data.time_limit);
	printf("CLI line\t: ' ");
	for (int a = 0; a < argc; a++) printf("%s ", argv[a]); 
	printf("'\n\n");
	#endif

	tsp_instance_from_cli(problem,&cli_data);
	
	uint64_t start_time = get_time();
	for(int i=0;i < problem->nnodes && start_time + cli_data.time_limit > get_time();i++){
		if(!strcmp(cli_data.method,"GREEDY"))	tsp_greedy(i,problem);
	}
	uint64_t end_time = get_time();

	print_best_solution_info(problem);
	if(problem->nnodes < 300) tsp_plot(problem,&cli_data);  
    
	#if VERBOSE > 0
	printf("TSP problem solved in %lu sec.s\n", end_time-start_time);  
	#endif
    
	instance_delete(problem);
	return 0; 
}
