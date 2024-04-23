#include "../include/tsp_exact.h"
#include "../include/tsp_solver.h"
#include "../include/tsp.h"
#include "../include/utils.h"
#include "../include/mincut.h"


#pragma region static_functions

/// @brief Set CPLEX log on
/// @param env CPLEX environment pointer
/// @param env TSPenv instance pointer
static void CPLEX_log(CPXENVptr* env,const TSPenv* tsp_env){
	CPXsetdblparam(*env, CPX_PARAM_SCRIND, CPX_OFF);
    char cplex_log_file[100];
    sprintf(cplex_log_file, "log/n_%u-%d-%s.log", tsp_env->random_seed, tsp_env->nnodes,tsp_env->method);
    if ( CPXsetlogfilename(*env, cplex_log_file, "w") ) print_error("CPXsetlogfilename error.\n");
}

/// @brief Set CPLEX log on
/// @param env CPLEX environment pointer
/// @param lp CPLEX model pointer
/// @param nnodes number of nodes
/// @param ncomp number of component
/// @param comp array that associate a number from 1 to n-component for each node
static void add_SEC(CPXCENVptr env, CPXLPptr lp,const unsigned int nnodes,const int ncomp,const int* comp){

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
		/*FIXME: Maybe I find a way: pointer to function
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


/// @brief Decompose the solution in the xstar format into n-component format
/// @param xstar solution in xstar format pointer
/// @param nnodes numbers of nodes
/// @param succ array of successor necessary to store the solution
/// @param comp array that associate a number from 1 to n-component for each node
/// @param ncomp number of component pointer
static void decompose_sol(const double *xstar, const unsigned int nnodes, int *succ, int *comp, int *ncomp){   
	#if VERBOSE > 2
		int *degree = (int *) calloc(nnodes, sizeof(int));
		for ( int i = 0; i < nnodes; i++ )
		{
			for ( int j = i+1; j < nnodes; j++ )
			{
				int k = coords_to_index(nnodes, i,j);
				if ( fabs(xstar[k]) > EPSILON && fabs(xstar[k]-1.0) > EPSILON ) print_error(" wrong xstar in decompose_sol()");
				if ( xstar[k] > 0.5 ) 
				{
					++degree[i];
					++degree[j];
				}
			}
		}
		for ( int i = 0; i < nnodes; i++ )
		{
			if ( degree[i] != 2 ) print_error("wrong degree in decompose_sol()");
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
		succ[i] = start;
	}
}


/// @brief Convert the solution saved on inst->solution to CPX format
/// @param inst TSPinst pointer
/// @param index array of indeces
/// @param value array of non-zeros
static void CPLEX_sol_from_inst(const TSPinst* inst,int* index, double* value){
		int nnz=0;
		int i;

		for(i = 0;i<inst->nnodes-1;i++){
				index[nnz]=coords_to_index(inst->nnodes,inst->solution[i],inst->solution[i+1]);
				value[nnz]=1.0;
				nnz++;
		}
		index[nnz]=coords_to_index(inst->nnodes,inst->solution[i],inst->solution[0]);
		value[nnz]=1.0;
}


/// @brief Create a CPLEX problem (env,lp) from a TSP instance
/// @param inst TSPinst instance pointer
/// @param env CPLEX environment pointer
/// @param lp CPLEX model pointer
static void CPLEX_model_new(TSPinst* inst, CPXENVptr* env, CPXLPptr* lp){    
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


/// @brief Delete a CPLEX problem (env,lp) 
/// @param env CPLEX environment pointer
/// @param lp CPLEX model pointer
static void CPLEX_model_delete(CPXENVptr* env, CPXLPptr* lp){
	CPXfreeprob(*env, lp);
	CPXcloseCPLEX(env); 
}


/// @brief Solve a CPLEX problem (env,lp) 
/// @param env CPLEX environment pointer
/// @param lp CPLEX model pointer
/// @param spare_time remaining time to compute a solution
/// @param lower_bound cost of solution 
/// @param x_star solution in CPX format
static int CPLEX_solve(CPXENVptr* env, CPXLPptr* lp, const double spare_time, double* lower_bound,double* x_star){

	if(spare_time < EPSILON) print_error("Time limit is too short!");
	
	CPXsetdblparam(*env,CPX_PARAM_TILIM,spare_time);
	if (CPXmipopt(*env,*lp)) print_error("CPXmipopt() error");

	int STATE = 0;
	switch(CPXgetstat(*env,*lp)){
		case CPXMIP_TIME_LIM_FEAS: 
			STATE=1; 
			print_warn("Time limit exceeded, but integer solution exists!"); //Time limit exceeded, but integer solution exists 
			break; 
		case CPXMIP_TIME_LIM_INFEAS: 		
			print_error("Time limit exceeded; no integer solution!"); //Time limit exceeded; no integer solution 
			break; 		 
		case CPXMIP_INFEASIBLE: 
			print_error("Solution is integer infeasible!"); //Solution is integer infeasible 
			break;
		default: break;	
	}

	CPXgetobjval(*env,*lp,lower_bound);
	if (CPXgetx(*env,*lp, x_star, 0, CPXgetnumcols(*env,*lp)-1)) print_error("CPXgetx() error");

	return STATE; //0 all good, 1 time_exceeding, but exist sol
}


/// @brief Callback function to add sec to cut pool
/// @param context callback context pointer
/// @param contextid context id
/// @param userhandle data passed to callback
static int CPXPUBLIC add_SEC_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* userhandle) { 

	if(contextid != CPX_CALLBACKCONTEXT_CANDIDATE) print_error("wrong callback");

	unsigned int nnodes = * (unsigned int*) userhandle;  
	
	int ncols = nnodes*(nnodes-1)/2;
	double* xstar = (double*) malloc(ncols * sizeof(double));  
	double objval = CPX_INFBOUND; 

	if (CPXcallbackgetcandidatepoint(context, xstar, 0, ncols-1, &objval)) print_error("CPXcallbackgetcandidatepoint error");

	int *succ = calloc(nnodes,sizeof(int));
	int *comp = calloc(nnodes,sizeof(int)); 
	int ncomp;

	decompose_sol(xstar,nnodes,succ,comp,&ncomp);
	free(xstar);

	if (ncomp == 1) {
		free(succ);
		free(comp);
		
		#if VERBOSE > 0
			double incumbent = CPX_INFBOUND;
			double l_bound = CPX_INFBOUND;
			CPXcallbackgetinfodbl(context,CPXCALLBACKINFO_BEST_SOL,&incumbent);
			CPXcallbackgetinfodbl(context,CPXCALLBACKINFO_BEST_BND,&l_bound);
			printf("\e[1mBRANCH & CUT\e[m found new feasible solution - Incumbent: %20.4f\tLower-Bound: %20.4f\tInt.Gap: %1.2f%% \n",incumbent,l_bound,(1-l_bound/incumbent)*100);
		#endif

		return 0;
	}
	

	//Add sec section
	int* index = calloc(ncols,sizeof(int));
	double* value = calloc(ncols,sizeof(double));
	
	char sense ='L';
	int start_index = 0;

	for(int k=1;k<=ncomp;k++) {
		int nnz=0;
		double rhs = -1.0;
		for(int i = 0;i < nnodes;i++){
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
		CPLEX_log(&CPLEX_env,env);
	#endif

	double init_time = get_time();

	//'Warm-up' CPLEX with a feasibile solution given by G2OPT heu
	if(env->warm){
		TSPgreedy(inst, ((double)rand())/RAND_MAX*inst->nnodes, TSPg2optb, env->method);
		print_warn("passing an heuristic solution to CPLEX...");

		int ncols = (inst->nnodes*(inst->nnodes-1))/2;
		int start_index = 0; 	
		int effort_level = CPX_MIPSTART_NOCHECK;
		int* index = (int*) calloc(ncols,sizeof(int));
		double* value = (double*) calloc(ncols,sizeof(double));

		CPLEX_sol_from_inst(inst,index,value);
		if (CPXaddmipstarts(CPLEX_env, CPLEX_lp, 1,ncols, &start_index, index, value, &effort_level, NULL)) print_error("CPXaddmipstarts() error");	
		free(index);
		free(value);
	}

	if(!strncmp(env->method,"BENDER", 6)) TSPCbenders(inst, env, &CPLEX_env,&CPLEX_lp);
	else if(!strncmp(env->method,"BRANCH_CUT", 12)) TSPCbranchcut(inst, env, &CPLEX_env,&CPLEX_lp);
	else { print_error("No function with alias"); }

	double final_time = get_time();

	#if VERBOSE > 0
		print_lifespan(final_time,init_time);
	#endif

	CPLEX_model_delete(&CPLEX_env,&CPLEX_lp);
} 


/// @brief use branch&cut to find a solution from LP solution
/// @param inst instance of TSPinst
/// @param tsp_env instance of TSPenv
/// @param env pointer to CPEXENVptr
/// @param lp pointer to CPEXLPptr
void TSPCbranchcut(TSPinst* inst, TSPenv* tsp_env, CPXENVptr* env, CPXLPptr* lp) {

	//Model has add_SEC_callback installed
	CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
	int nnodes = inst->nnodes;
	if (CPXcallbacksetfunc(*env, *lp, contextid, add_SEC_callback, &nnodes)) print_error("CPXcallbacksetfunc() error");

	double start_time = get_time(); 
	double lb = inst->cost;

	int *succ = calloc(inst->nnodes,sizeof(int));
	int *comp = calloc(inst->nnodes,sizeof(int)); 
	int ncomp;

	double* x_star = (double*) calloc((inst->nnodes*(inst->nnodes-1))/2, sizeof(double));
	CPLEX_solve(env,lp,tsp_env->time_limit-time_elapsed(start_time),&lb,x_star);
			
	decompose_sol(x_star,inst->nnodes,succ,comp,&ncomp);
	free(x_star);

	if(lb < inst->cost){
		int* sol  = calloc(inst->nnodes,sizeof(int));
		if(ncomp != 1){
			strcpy(tsp_env->method,"B&C-PATCHING");
			patching(inst, succ, comp, ncomp);
		} 

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
	double lb = inst->cost;

	int* succ = calloc(inst->nnodes,sizeof(int));
	int* comp = calloc(inst->nnodes,sizeof(int)); 
	int ncomp;
	int iter=0;

	double start_time = get_time(); 
	if(tsp_env->time_limit-time_elapsed(start_time)<EPSILON) print_error("Time limit is too short!");

	while(time_elapsed(start_time) < tsp_env->time_limit) {
		iter++;

		double* x_star = (double*) calloc((inst->nnodes*(inst->nnodes-1))/2, sizeof(double));
		CPLEX_solve(env,lp,tsp_env->time_limit-time_elapsed(start_time),&lb,x_star);

		#if VERBOSE > 0
			printf("Lower-Bound \e[1mBENDER'S LOOP\e[m itereation [%i]: \t%10.4f\n", iter, lb);
		#endif

		decompose_sol(x_star,inst->nnodes,succ,comp,&ncomp);
		free(x_star);

		//Iter = 0 --> BENDERS reaches the end
		if(ncomp == 1){
			iter=0;
			break;
		}

		//We always apply patching on Benders, in order to have solution if we exceed tl
		add_SEC(*env,*lp,inst->nnodes,ncomp,comp);
		patching(inst,succ,comp,ncomp);
	}

	if(lb < inst->cost){
		if(iter) strcpy(tsp_env->method,"BENDERS-PATCHING");

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
void patching(TSPinst* inst, int* succ, int* comp, const unsigned int comp_size) {

	int best_i = 0, best_j = 0;
	int best_set = 0;

	int group_size = comp_size;

	int kgroupp[group_size];
	memset(kgroupp, 0, comp_size * sizeof(int));
	int h = 0;
	for(int l = 0; h < comp_size; l++) {
		if(kgroupp[comp[l]-1] != 0) continue;
		kgroupp[comp[l]-1] = succ[l];
		h++;
	}

	while(group_size > 1) {
		for(int k1 = 0; k1 < group_size-1; k1++) {
			double min = DBL_MAX;
			for(int k2 = k1+1; k2 < group_size; k2++) {

				int i = kgroupp[k1];
				int j = kgroupp[k2];
				while (succ[i] != kgroupp[k1]) {
					while (succ[j] != kgroupp[k2]) {

						double del_cost = delta_cost(inst, j, succ[j], i, succ[i]);
						if(del_cost < min) {
							min = del_cost;
							best_i = i;
							best_j = j;
							best_set = k2;
						}
						j = succ[j];
					}
					i = succ[i];
				}
				
			}

			#if VERBOSE > 2
				printf("\n=============INFO=============\n");
				printf("tour n°: %i\n", comp[kgroupp[k1]]);
				printf("min:\t%10.4f\ni:\t\t%i\nsucc[i]:\t%i\nj:\t\t%i\nsucc[j]:\t%i\nbest_set:\t%i\n", min, best_i, succ[best_i], best_j, succ[best_j], comp[kgroupp[best_set]]);
				printf("==============================\n");
			#endif

			inst->cost += min;
			int dummy = succ[best_i];
			succ[best_i] = succ[best_j];
			succ[best_j] = dummy;

			int z = kgroupp[best_set]; 
			while(succ[z] != kgroupp[best_set]) {
				comp[z] = comp[kgroupp[k1]];
				z = succ[z];
			}
			kgroupp[best_set] = kgroupp[--group_size];

			#if VERBOSE > 2
				if(arrunique(succ, inst->nnodes) != inst->nnodes) perror("error in solution building");
				printf("\nremaining tour: %i", comp[kgroupp[0]]);
				for(int i = 1 ; i < group_size; i++) {
				printf(",%i", comp[kgroupp[i]]);
				}
				printf("\n");
			#endif

		}
	}
}