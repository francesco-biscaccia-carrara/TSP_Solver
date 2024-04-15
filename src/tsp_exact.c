#include "../include/tsp_exact.h"
#include "../include/tsp.h"
#include "../include/utils.h"


#pragma region static_functions

void add_sec(CPXCENVptr env, CPXLPptr lp,TSPinst* problem,int ncomp,int ncols,int* comp){
	if(ncomp==1) print_error("no sec needed for 1 comp!");

	int* index = (int*) calloc(ncols,sizeof(int));
	double* value = (double*) calloc(ncols,sizeof(double));

	for(int k=1;k<=ncomp;k++){
		int nnz=0;
		char sense ='L';
		int start_index = 0;
		double rhs = -1.0;
		for(int i =0;i<problem->nnodes;i++){
			if(comp[i]!=k) continue;
			rhs++;
			for(int j =i+1;j < problem->nnodes;j++){
				if(comp[j]!=k) continue;
				index[nnz]=coords_to_index(problem->nnodes,i,j);
				value[nnz]=1.0;
				nnz++;
			}
		}
		CPXaddrows(env,lp,0,1,nnz,&rhs,&sense,&start_index,index,value,NULL,NULL);
	}

	free(index);
	free(value);
}

void build_model(TSPinst *problem, CPXENVptr env, CPXLPptr lp){    
	int izero = 0;
	char binary = 'B'; 
    double lb = 0.0;
	double ub = 1.0;

	char **cname = (char **) calloc(1, sizeof(char *));	
	cname[0] = (char *) calloc(100, sizeof(char));

	// add binary var.s x(i,j) for i < j  
	for ( int i = 0; i < problem->nnodes; i++ ){
		for ( int j = i+1; j < problem->nnodes; j++ ){
			sprintf(cname[0], "x(%d,%d)", i+1,j+1);  
			double obj = get_arc(problem,i,j);
			if ( CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname) ){
                printf("%d",CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname));
                print_error("wrong CPXnewcols on x var.s");
            }
            
    		if ( CPXgetnumcols(env,lp)-1 != coords_to_index(problem->nnodes,i,j) ) print_error("wrong position for x var.s");
		}
	} 

	int *index = (int *) malloc(problem->nnodes * sizeof(int));
	double *value = (double *) malloc(problem->nnodes * sizeof(double));  
	
	// add the degree constraints
	for ( int h = 0; h < problem->nnodes; h++ )  {
		double rhs = 2.0;
		char sense = 'E';                     // 'E' for equality constraint 
		sprintf(cname[0], "degree(%d)", h+1); 
		int nnz = 0;
		for ( int i = 0; i < problem->nnodes; i++ ){
			if ( i == h ) continue;
			index[nnz] = coords_to_index(problem->nnodes,h, i);
			value[nnz] = 1.0;
			nnz++;
		}
		
		if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]) ) print_error(" wrong CPXaddrows [degree]");
	} 

    free(value);
    free(index);	

    #if VERBOSE > 1
	CPXwriteprob(env, lp, "model.lp", NULL);   
    #endif

	free(cname[0]);
	free(cname);

}

int xpos(int i, int j, TSPinst *inst)      // to be verified                                           
/***************************************************************************************************************************/
{ 
	if ( i == j ) print_error(" i == j in xpos" );
	if ( i > j ) return xpos(j,i,inst);
	int pos = i * inst->nnodes + j - (( i + 1 ) * ( i + 2 )) / 2;
	return pos;
}

void build_sol(const double *xstar, TSPinst *inst, int *succ, int *comp, int *ncomp){   
	#if VERBOSE > 2
		int *degree = (int *) calloc(inst->nnodes, sizeof(int));
		for ( int i = 0; i < inst->nnodes; i++ )
		{
			for ( int j = i+1; j < inst->nnodes; j++ )
			{
				int k = xpos(i,j,inst);
				if ( fabs(xstar[k]) > EPSILON && fabs(xstar[k]-1.0) > EPSILON ) print_error(" wrong xstar in build_sol()");
				if ( xstar[k] > 0.5 ) 
				{
					++degree[i];
					++degree[j];
				}
			}
		}
		for ( int i = 0; i < inst->nnodes; i++ )
		{
			if ( degree[i] != 2 ) print_error("wrong degree in build_sol()");
		}	
		free(degree);
	#endif

	*ncomp = 0;
	for ( int i = 0; i < inst->nnodes; i++ ) {
		succ[i] = -1;
		comp[i] = -1;
	}
	
	for ( int start = 0; start < inst->nnodes; start++ )
	{
		if ( comp[start] >= 0 ) continue;  // node "start" was already visited, just skip it

		// a new component is found
		(*ncomp)++;
		int i = start;
		int done = 0;
		while ( !done )  // go and visit the current component
		{
			comp[i] = *ncomp;
			done = 1;
			for ( int j = 0; j < inst->nnodes; j++ )
			{
				if ( i != j && xstar[coords_to_index(inst->nnodes,i,j)] > 0.5 && comp[j] == -1 ) // the edge [i,j] is selected in xstar and j was not visited before 
				{
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

#pragma endregion

int tsp_CPX_opt(TSPinst *problem){  
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); 

	build_model(problem, env, lp);

	if (  CPXmipopt(env,lp)) print_error("CPXmipopt() error");    
    

	int ncols = CPXgetnumcols(env, lp);
	double sol_cost = 0;
	double best_obj;
	double *xstar = (double *) calloc(ncols, sizeof(double));
	if (CPXgetx(env, lp, xstar, 0, ncols-1)) print_error("CPXgetx() error");	
	for ( int i = 0; i < problem->nnodes; i++ ){
		for ( int j = i+1; j < problem->nnodes; j++ )
			if (xstar[coords_to_index(problem->nnodes,i,j)] > 0.5) {
				sol_cost += get_arc(problem,i,j);
				printf("  ... x(%3d,%3d) = 1\n", i+1,j+1);
			}
		
	}

	//Cost check
	CPXgetbestobjval(env,lp,&best_obj);
	if(sol_cost > best_obj + 1e-7 || sol_cost < best_obj - 1e-7) print_error("EZ");

	if(sol_cost < problem->cost){
		problem->cost = sol_cost;
	}

	free(xstar);
	
	// free and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 

	return 0; // or an appropriate nonzero error code

}

void tsp_bender_loop(TSPinst* problem, TSPenv* cli){
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); 

	build_model(problem, env, lp);
	double start_time = get_time(); 
	double lb = problem->cost;

	int *succ= calloc(problem->nnodes,sizeof(int));
	int *sol= calloc(problem->nnodes,sizeof(int));
	int *comp = calloc(problem->nnodes,sizeof(int)); 
	int ncomp;
	int iter=0;

	while(iter < 3){
		iter++;
		CPXsetdblparam(env,CPX_PARAM_TILIM,cli->time_limit-time_elapsed(start_time));
		if (CPXmipopt(env,lp)) print_error("CPXmipopt() error");
		CPXgetbestobjval(env,lp,&lb);
		
		#if VERBOSE > 0
		printf("Iter: %3d\tCost: %10.4f\n",iter,lb);
		#endif

		int ncols = CPXgetnumcols(env, lp);
		double *xstar = (double *) calloc(ncols, sizeof(double));
		if (CPXgetx(env, lp, xstar, 0, ncols-1)) print_error("CPXgetx() error");
		build_sol(xstar,problem,succ,comp,&ncomp);

		free(xstar);
		if(ncomp==1) break;
		add_sec(env,lp,problem,ncomp,ncols,comp);
		if(time_elapsed(start_time) > cli->time_limit) break;
	}

	if(lb < problem->cost){
		problem->cost=lb;
		
		//Update incumbent
		if( ncomp == 1){
			cth_convert(sol, succ, problem->nnodes);
			problem->solution=sol;

		}
		else{			
			patching(problem, succ, comp);
			cth_convert(sol, succ, problem->nnodes);
			problem->solution=sol;	
		}	
	}

	free(succ);
	free(comp);
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 
}

void patching(TSPinst* inst, int* succ, int* comp) {

	double min = DBL_MAX;
	int best_i = 0, best_j = 0;
	int best_set = 0;

	int group_size = arrunique(comp, inst->nnodes);
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

			#if VERBOSE > 1
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

			#if VERBOSE > 1
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