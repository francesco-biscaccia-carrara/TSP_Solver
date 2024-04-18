#include "../include/tsp_exact.h"
#include "../include/tsp.h"
#include "../include/utils.h"


#pragma region static_functions
//TODO: END DOCS

void CPLEX_log(CPXENVptr* env,unsigned int seed,unsigned int nnodes,const char* method){
	CPXsetdblparam(*env, CPX_PARAM_SCRIND, CPX_OFF);
    char cplex_log_file[100];
    sprintf(cplex_log_file, "log/%u-%d-%s.log", seed, nnodes,method);
    if ( CPXsetlogfilename(*env, cplex_log_file, "w") ) print_error("CPXsetlogfilename error.\n");
}

void add_sec(CPXCENVptr env, CPXLPptr lp,const unsigned int nnodes,const int ncomp,const int* comp){

	if(ncomp==1) print_error("no sec needed for 1 comp!");

	int* index = (int*) calloc((nnodes*(nnodes-1))/2,sizeof(int));
	double* value = (double*) calloc((nnodes*(nnodes-1))/2,sizeof(double));
	
	char sense ='L';
	int start_index = 0;

	for(int k=1;k<=ncomp;k++) {
		int nnz=0;
		double rhs = -1.0;
		for(int i = 0;i<nnodes;i++){
			if(comp[i]!=k) continue;
			rhs++;
			for(int j =i+1;j < nnodes;j++){
				if(comp[j]!=k) continue;
				index[nnz]=coords_to_index(nnodes,i,j);
				value[nnz]=1.0;
				nnz++;
			}
		}
		if( CPXaddrows(env,lp,0,1,nnz,&rhs,&sense,&start_index,index,value,NULL,NULL)) print_error("CPXaddrows() error");
		/*FIXME: Must exist a way to do that!
		...CPXCALLBACKCONTEXTptr contextid, CPXCENVptr env, CPXLPptr lp
		if(env != NULL && lp !=NULL) 
			if( CPXaddrows(env,lp,0,1,nnz,&rhs,&sense,&start_index,index,value,NULL,NULL)) print_error("CPXaddrows() error");
		else if(contextid != NULL) 
			if ( CPXcallbackrejectcandidate(contextid, 1, nnz, &rhs, &sense, &start_index, index, value) ) print_error("CPXcallbackrejectcandidate() error"); 
		else print_error("wrong use of function");
		*/
	}
	free(index);
	free(value);
}

void rewrite_solution(const double *xstar, const unsigned int nnodes, int *succ, int *comp, int *ncomp){   
	#if VERBOSE > 2
		int *degree = (int *) calloc(nnodes, sizeof(int));
		for ( int i = 0; i < nnodes; i++ )
		{
			for ( int j = i+1; j < nnodes; j++ )
			{
				int k = coords_to_index(nnodes, i,j);
				if ( fabs(xstar[k]) > EPSILON && fabs(xstar[k]-1.0) > EPSILON ) print_error(" wrong xstar in rewrite_solution()");
				if ( xstar[k] > 0.5 ) 
				{
					++degree[i];
					++degree[j];
				}
			}
		}
		for ( int i = 0; i < nnodes; i++ )
		{
			if ( degree[i] != 2 ) print_error("wrong degree in rewrite_solution()");
		}	
		free(degree);
	#endif

	*ncomp = 0;
	for ( int i = 0; i < nnodes; i++ ) {
		succ[i] = -1;
		comp[i] = -1;
	}
	
	for ( int start = 0; start < nnodes; start++ ){
		if ( comp[start] >= 0 ) continue;  // node "start" was already visited, just skip it
		// a new component is found
		(*ncomp)++;
		int i = start;
		int done = 0;
		while ( !done ){
			comp[i] = *ncomp;
			done = 1;
			for ( int j = 0; j < nnodes; j++ ){
				if ( i != j && xstar[coords_to_index(nnodes,i,j)] > 0.5 && comp[j] == -1 ) {// the edge [i,j] is selected in xstar and j was not visited before
					succ[i] = j;
					i = j;
					done = 0;
					break;
				}
			}
		}	
		succ[i] = start;  // last arc to close the cycle
		
		// go to the next component...
	}
}

void CPLEX_model_new(TSPinst* inst, CPXENVptr* env, CPXLPptr* lp){    
	//Env and empty model created
	int error;
	*env = CPXopenCPLEX(&error);
	*lp  = CPXcreateprob(*env, &error, "TSP"); 
	if(error) print_error("model not created");

	int izero = 0;
	char binary = 'B'; 
    double lb = 0.0;
	double ub = 1.0;

	char **cname = (char **) calloc(1, sizeof(char *));	
	cname[0] = (char *) calloc(100, sizeof(char));

	// add binary var.s x(i,j) for i < j  
	for ( int i = 0; i < inst->nnodes; i++ ){
		for ( int j = i+1; j < inst->nnodes; j++ ){

			sprintf(cname[0], "x(%d,%d)", i+1,j+1);  
			double obj = get_arc(inst,i,j);

			if ( CPXnewcols(*env, *lp, 1, &obj, &lb, &ub, &binary, cname) ) print_error("wrong CPXnewcols on x var.s");
        	if ( CPXgetnumcols(*env,*lp)-1 != coords_to_index(inst->nnodes,i,j) ) print_error("wrong position for x var.s");
		}
	} 

	int *index = (int *) malloc(inst->nnodes * sizeof(int));
	double *value = (double *) malloc(inst->nnodes * sizeof(double));  
	
	// add the degree constraints
	for ( int h = 0; h < inst->nnodes; h++ )  {
		double rhs = 2.0;
		char sense = 'E';                     // 'E' for equality constraint 
		sprintf(cname[0], "degree(%d)", h+1); 
		int nnz = 0;
		for ( int i = 0; i < inst->nnodes; i++ ){
			if ( i == h ) continue;
			index[nnz] = coords_to_index(inst->nnodes,h, i);
			value[nnz] = 1.0;
			nnz++;
		}
		
		if (CPXaddrows(*env, *lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]) ) print_error(" wrong CPXaddrows [degree]");
	} 

    free(value);
    free(index);
	free(cname[0]);
	free(cname);
}

void CPLEX_model_delete(CPXENVptr* env, CPXLPptr* lp){
	CPXfreeprob(*env, lp);
	CPXcloseCPLEX(env); 
}

/// @brief Solve a CPLEX problem (env,lp) 
/// @param env CPLEX environment pointer
/// @param lp CPLEX model pointer
/// @param spare_time remaining time to compute a solution
/// @param lower_bound cost of solution 
/// @param x_star solution in CPX format
void CPLEX_solve(CPXENVptr* env, CPXLPptr* lp, const double spare_time, double* lower_bound,double* x_star){

	//TODO: handle infeas, time_exceed cases
	CPXsetdblparam(*env,CPX_PARAM_TILIM,spare_time);
	if (CPXmipopt(*env,*lp)) print_error("CPXmipopt() error");

	CPXgetobjval(*env,*lp,lower_bound);

	if (CPXgetx(*env,*lp, x_star, 0, CPXgetnumcols(*env,*lp)-1)) print_error("CPXgetx() error");
}

/// @brief Callback function to add sec to cut pool
/// @param context callback context pointer
/// @param contextid context id
/// @param userhandle data passed to callback
static int CPXPUBLIC add_sec_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* userhandle) { 
	unsigned int nnodes = * (unsigned int*) userhandle;  
	
	int ncols = nnodes*(nnodes-1)/2;
	double* xstar = (double*) malloc(ncols * sizeof(double));  
	double objval = CPX_INFBOUND; 

	if (CPXcallbackgetcandidatepoint(context, xstar, 0, ncols-1, &objval)) print_error("CPXcallbackgetcandidatepoint error");

	int *succ = calloc(nnodes,sizeof(int));
	int *comp = calloc(nnodes,sizeof(int)); 
	int ncomp;

	rewrite_solution(xstar,nnodes,succ,comp,&ncomp);
	free(xstar);

	if (ncomp == 1) return 0;

	//Add sec section
	int* index = (int*) calloc((nnodes*(nnodes-1))/2,sizeof(int));
	double* value = (double*) calloc((nnodes*(nnodes-1))/2,sizeof(double));
	
	char sense ='L';
	int start_index = 0;

	for(int k=1;k<=ncomp;k++) {
		int nnz=0;
		double rhs = -1.0;
		for(int i = 0;i<nnodes;i++){
			if(comp[i]!=k) continue;
			rhs++;
			for(int j =i+1;j < nnodes;j++){
				if(comp[j]!=k) continue;
				index[nnz]=coords_to_index(nnodes,i,j);
				value[nnz]=1.0;
				nnz++;
			}
		}
		
		if (CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &start_index, index, value) ) print_error("CPXcallbackrejectcandidate() error"); 
	
	}
	free(index);
	free(value);

	free(succ);
	free(comp);
	return 0; 
}

#pragma endregion

/// @brief Solve an instance of TSP with a mixed exact approach
/// @param inst instance of TSPinst
/// @param env instance  of TSPenv
void TSPCsolve(TSPinst* inst, TSPenv* env) {
	
	CPXENVptr CPLEX_env = NULL; 
	CPXLPptr CPLEX_lp = NULL;
	CPLEX_model_new(inst, &CPLEX_env, &CPLEX_lp);

	#if VERBOSE > 1
		CPLEX_log(&CPLEX_env,env->random_seed,inst->nnodes,env->method);
	#endif

	double init_time = get_time();

	if(!strncmp(env->method,"BENDERS", 7)) TSPCbenders(inst, env, &CPLEX_env,&CPLEX_lp);
	else if(!strncmp(env->method,"BRANCH_BOUND", 12)) TSPCbranchbound(inst, env, &CPLEX_env,&CPLEX_lp);
	else { print_error("No function with alias"); }

	CPLEX_model_delete(&CPLEX_env,&CPLEX_lp);

	double final_time = get_time();

    #if VERBOSE > 0
		printf("TSP problem solved in %10.4f sec.s\n", final_time-init_time);
	#endif
} 

/// @brief use branch&cut to find a solution from LP solution
/// @param inst instance of TSPinst
/// @param tsp_env instance of TSPenv
/// @param env pointer to CPEXENVptr
/// @param lp pointer to CPEXLPptr
void TSPCbranchbound(TSPinst* inst, TSPenv* tsp_env, CPXENVptr* env, CPXLPptr* lp) {

	//Model has add_sec_callback installed
	CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
	int nnodes = inst->nnodes;
	if (CPXcallbacksetfunc(*env, *lp, contextid, add_sec_callback, &nnodes)) print_error("CPXcallbacksetfunc() error");

	double start_time = get_time(); 
	double lb = inst->cost;

	int *succ = calloc(inst->nnodes,sizeof(int));
	int *comp = calloc(inst->nnodes,sizeof(int)); 
	int ncomp;

	double* x_star = (double*) calloc((inst->nnodes*(inst->nnodes-1))/2, sizeof(double));
	CPLEX_solve(env,lp,tsp_env->time_limit-time_elapsed(start_time),&lb,x_star);

	rewrite_solution(x_star,inst->nnodes,succ,comp,&ncomp);
	free(x_star);

	if(lb < inst->cost){
		int* sol  = calloc(inst->nnodes,sizeof(int));
		double cost = compute_cost(inst,cth_convert(sol, succ, inst->nnodes));
		instance_set_solution(inst,sol,cost);
	}

	free(succ);
	free(comp);

}

/// @brief use bender's loop to solve a solution from LP solution
/// @param inst instance of TSPinst
/// @param tsp_env instance of TSPenv
/// @param env pointer to CPEXENVptr
/// @param lp pointer to CPEXLPptr
void TSPCbenders(TSPinst* inst, TSPenv* tsp_env, CPXENVptr* env, CPXLPptr* lp) {	
	double start_time = get_time(); 
	double lb = inst->cost;

	int* succ = calloc(inst->nnodes,sizeof(int));
	int* comp = calloc(inst->nnodes,sizeof(int)); 
	int ncomp;
	int iter=0;

	while(time_elapsed(start_time) < tsp_env->time_limit) {
		iter++;

		double* x_star = (double*) calloc((inst->nnodes*(inst->nnodes-1))/2, sizeof(double));
		CPLEX_solve(env,lp,tsp_env->time_limit-time_elapsed(start_time),&lb,x_star);

		#if VERBOSE > 0
			printf("Lower-bound \e[1mBENDER'S LOOP\e[m itereation [%i]: \t%10.4f\n", iter, lb);
		#endif

		rewrite_solution(x_star,inst->nnodes,succ,comp,&ncomp);
		free(x_star);

		if(ncomp==1) break;

		//We always apply patching on Benders, in order to have solution if we exceed tl
		add_sec(*env,*lp,inst->nnodes,ncomp,comp);
		patching(inst,succ,comp,ncomp);
		if(time_elapsed(start_time) > tsp_env->time_limit) break;
	}

	if(lb < inst->cost){
		if(ncomp != 1) patching(inst, succ, comp, ncomp);

		int* sol  = calloc(inst->nnodes,sizeof(int));
		double cost = compute_cost(inst,cth_convert(sol, succ, inst->nnodes));
		instance_set_solution(inst,sol,cost);
	}

	free(succ);
	free(comp);
}


/// @brief patching for a non final bender's loop solution 
/// @param inst instance of TSPinst
/// @param succ solution in cplex format
/// @param comp array that associate edge with route number
/// @param comp_size number of unique element into comp array
void patching(TSPinst* inst, int* succ, int* comp, const unsigned int comp_size) { //FIXME: #Rick - maybe use local structu??

	double min = DBL_MAX;
	int best_i = 0, best_j = 0;
	int best_set = 0;

	int group_size = comp_size;
	int kgroup[group_size];
	for(int o = 0; o < group_size; o++) kgroup[o] = o+1;


	while(group_size > 1) {
		for(int k1 = 0; k1 < group_size-1; k1++) {
			for(int k2 = k1+1; k2 < group_size; k2++) {
				for(int i = 0; i < inst->nnodes; i++) {
					if(comp[i] != kgroup[k1]) continue;

					for(int j = 0; j < inst->nnodes; j++) {
						if(comp[j] != kgroup[k2]) continue;

						double del_cost = delta_cost(inst, j, succ[j], i, succ[i]);
						if(del_cost < min) {
							min = del_cost;
							best_i = i;
							best_j = j;
							best_set = k2;
						}

					}
				}
			}

			#if VERBOSE > 2
				printf("\n=============INFO=============\n");
				printf("tour n°: %i\n", kgroup[k1]);
				printf("min:\t%10.4f\ni:\t\t%i\nsucc[i]:\t%i\nj:\t\t%i\nsucc[j]:\t%i\nbest_set:\t%i\n", min, best_i, succ[best_i], best_j, succ[best_j], kgroup[best_set]);
				printf("==============================\n");
			#endif

			inst->cost += min;
			int dummy = succ[best_i];
			succ[best_i] = succ[best_j];
			succ[best_j] = dummy;

			for(int z = 0; z < inst->nnodes; z++) {
				if(comp[z] == kgroup[best_set]) comp[z] = kgroup[k1];
			}
			kgroup[best_set] = kgroup[--group_size];

			#if VERBOSE > 2
				if(arrunique(succ, inst->nnodes) != inst->nnodes) perror("error in solution building");
				printf("\nremaining tour: %i", kgroup[0]);
				for(int i = 1 ; i < group_size; i++) {
				printf(",%i", kgroup[i]);
				}
				printf("\n");
			#endif

			min = DBL_MAX;
		}
	}
}