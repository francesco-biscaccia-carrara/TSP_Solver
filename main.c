#include "include/utils.h"
#include "include/tsp.h"

#define MAX_NODES_PLOT 300
#define MAX_TIME    3.6e+6	//An hour

int main(int argc, char **argv){
    if (argc < 2) {
		printf("Usage: %s -h/ -help/ --help/  to get some help\n", argv[0]);
		exit(1);
	} 

	uint64_t time_limit= MAX_TIME;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i],"-tl") ||!strcmp(argv[i],"-max_time")) time_limit = abs(atoi(argv[++i])+1);
	} 

	#if VERBOSE > 1
	printf("__log: Time_limit: %llu ms\n",time_limit);
	printf("__log: CLI: ' ");
	for (int a = 0; a < argc; a++) printf("%s ", argv[a]); 
	printf("'\n");
	#endif

	instance* problem=instance_new();

	tsp_instance_from_cli(argc,argv, problem);
	
	uint64_t start_time = get_time();
	for(int i=0;i < problem->nnodes && start_time + time_limit > get_time();i++){
		tsp_greedy(i,problem);
	}
	uint64_t end_time = get_time();

	print_best_solution_info(problem);
	if(problem->nnodes < 300) tsp_plot(problem);  
    
	#if VERBOSE > 9 
	printf("__log: TSP problem solved in %llu sec.s\n", end_time-start_time);  
	#endif
    
	instance_delete(problem);
	return 0; 
}
