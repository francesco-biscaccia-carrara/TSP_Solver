#include "include/algorithm.h"
#include "include/display.h"
#include "include/load.h"
#include "include/tsp.h"

#define TESTPHASE 0

typedef struct {
	double cost;
	int seed;
} out;

out procedure (int argc, char **argv) {
	out ret = {.cost = 0, .seed=10};
	if (argc < 2) {
		printf("Usage: %s -h/ -help/ --help/  to get some help\n", argv[0]);
		exit(1);
	} 

	cli_info cli_data;
	parse_cli(argc,argv,&cli_data);

	//instance* problem=instance_new();

	signal(SIGINT, check_signal);

	#if VERBOSE > 1
	printf("Time Limit\t: %lu ms\n",cli_data.time_limit);
	printf("CLI line\t: ' ");
	for (int a = 0; a < argc; a++) printf("%s ", argv[a]); 
	printf("'\n\n");
	#endif

	instance* problem = instance_new_cli(&cli_data);
	//tsp_instance_from_cli(problem,&cli_data);
	
	solve_heuristic(&cli_data, problem);
	print_best_solution_info(problem,&cli_data);
	tsp_plot(problem,&cli_data);  
	instance_delete(problem);

	ret.cost = problem->cost;
	ret.seed = cli_data.random_seed;

	return ret;
}


#if TESTPHASE == 1
int main(int argc, char **argv){

	FILE *f = fopen("performance_python/data_comparison.csv", "w");
	char* ss[3] = {"VNS_B", "VNS_R"};

	fprintf(f, "2, VNS_B, VNS_R\n");
	for(int i = 0; i < 10; i++) {
		sprintf(argv[6], "%i", i);
		printf("SEED:%s\n", argv[6]);
		fprintf(f, "%i", i);
		for(int j = 0; j < 2; j++) {
			argv[8] = ss[j];
			printf("PROCEDURE %s\n", argv[8]);
			out proc = procedure(argc, argv);
			fprintf(f, ", %10.4f",proc.cost);
		}
		fprintf(f, "\n");

	} 
	system("cd performance_python && python3.10 perfprof.py -D , -T 3 -M 2 data_comparison.csv pp.pdf -P \"all instances, shift 2 sec.s\"");
	return 0; 
}
#else
int main(int argc, char **argv){
	out proc = procedure(argc, argv);
	return 0; 
}
#endif