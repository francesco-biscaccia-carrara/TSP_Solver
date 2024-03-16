#include "include/algorithm.h"
#include "include/display.h"
#include "include/load.h"
#include "include/tsp.h"


int main(int argc, char **argv){
    if (argc < 2) {
		printf("Usage: %s -h/ -help/ --help/  to get some help\n", argv[0]);
		exit(1);
	} 

	cli_info cli_data;
	parse_cli(argc,argv,&cli_data);

	instance* problem=instance_new();

	#if VERBOSE > 1
	printf("Time Limit\t: %lu ms\n",cli_data.time_limit);
	printf("CLI line\t: ' ");
	for (int a = 0; a < argc; a++) printf("%s ", argv[a]); 
	printf("'\n\n");
	#endif

	tsp_instance_from_cli(problem,&cli_data);
	
	solve_heuristic(&cli_data, problem);
	print_best_solution_info(problem,&cli_data);
	tsp_plot(problem,&cli_data);  
    
	instance_delete(problem);
	return 0; 
}
