#include "utils.h"

int main(int argc, char **argv){
    if (argc < 2) {printf("Usage: %s -h/ -help/ --help/  to get some help\n", argv[0]); exit(1);}       
	if (VERBOSE >= 10) {for (int a = 0; a < argc; a++) printf("__log: %s ", argv[a]); printf("\n");}

	uint64_t t1 = get_time(); 
	instance inst;

	parse_cli(argc,argv, &inst);
	
	read_tsp_file(&inst);
	plot(&inst);  
	//if (VRPopt(&inst)) print_error(" error within VRPopt()");
	uint64_t t2 = get_time(); 
    
	if(VERBOSE >= 10) printf("__log: TSP problem solved in %lu sec.s\n", t2-t1);  
    
	free_instance(&inst);
	return 0; 
}