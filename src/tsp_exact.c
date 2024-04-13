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

	while(1){
		iter++;
		CPXsetdblparam(env,CPX_PARAM_TILIM,cli->time_limit-time_elapsed(start_time));
		if (CPXmipopt(env,lp)) print_error("CPXmipopt() error");   
		CPXgetbestobjval(env,lp,&lb);
		//Print incumbent cost
		printf("Iter: %3d\tCost: %10.4f\n",iter,lb);
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
			//check_tour_cost(problem, problem->solution, problem->cost);
			int prev = 0;
			for(int i = 0; i < problem->nnodes; i++) {printf("|%i", succ[i]);}
			printf("\n\n");
			for(int i = 0; i < problem->nnodes; i++) {printf("%i->", succ[prev]);
			prev = succ[prev]; }
			printf("\n\n");

			double cost = 0;
			for(int i = 0; i < problem->nnodes-1; i++) {
				cost += get_arc(problem, problem->solution[i], problem->solution[i+1]); 
			}
			cost += get_arc(problem, problem->solution[0], problem->solution[problem->nnodes-1]);
			printf("computed cost: %10.4f\n", cost);	

		}
		else{			
			patching(problem, succ, comp);
			cth_convert(sol, succ, problem->nnodes);
			problem->solution=sol;

			double cost1 = 0;
			for(int i = 0; i < problem->nnodes-1; i++) {
				cost1 += get_arc(problem, problem->solution[i], problem->solution[i+1]); 
			}
			cost1 += get_arc(problem, problem->solution[0], problem->solution[problem->nnodes]);

			printf("computed cost: %10.4f\n", cost1);	
		}	
	}

	free(succ);
	free(comp);

	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 
}

static double delta_cost(TSPinst* inst, const unsigned int j, const unsigned int jp, 
										const unsigned int i, const unsigned int ip) {

	return (get_arc(inst, i, jp) + get_arc(inst, j, ip)) - (get_arc(inst, i, ip) + get_arc(inst, j, jp));

}

void patching(TSPinst* inst, int* succ, int* comp) {
	int groups = arrunique(comp, inst->nnodes);
	double min = 10e+8;
	int best_q = 0,best_qnext = 0;
	int best_s = 0,best_snext = 0;
	int best_set = 0;

	int kgroup[groups];
	for(int o = 0; o < groups; o++) kgroup[o] = o+1;


	while(groups > 1) {
	
	for(int k1 = 0; k1 < groups-1; k1++) {
		printf("K = %i\n", kgroup[k1]);
		for(int k2 = k1+1; k2 < groups; k2++) {
			if(kgroup[k1] == kgroup[k2]) { continue; }
			for(int i = 0; i < inst->nnodes; i++) {
				if(comp[i] != kgroup[k1]) continue;

				for(int j = 0; j < inst->nnodes; j++) {
					if(comp[j] != kgroup[k2]) continue;

					double dc = delta_cost(inst, j, succ[j], i, succ[i]);
					if(dc < min) {
						min = dc;
						best_q = i;
						best_s = j;
						best_qnext = succ[i];
						best_snext = succ[j];
						best_set = k2;
					}

				}
			}
		}
		printf("INFO\nmin:%10.4f\nq:%i\nqn:%i\ns:%i\nsn:%i\nbest_set:%i\n", min, best_q, best_qnext, best_s, best_snext, kgroup[best_set]);

		inst->cost += min;
		succ[best_q] = best_snext;
		succ[best_s] = best_qnext;
		min = 10e+8;

		for(int z = 0; z < inst->nnodes; z++) {
			if(comp[z] == kgroup[best_set]) {
				comp[z] = kgroup[k1];
			}
		}

		kgroup[best_set] = kgroup[groups-1];

		groups--;
		if(arrunique(succ, inst->nnodes) != inst->nnodes) perror("ecco vedi istanza sbagliata, l'avevo detto, sei un pirla, vergognati");

		printf("\nKGROUP\n");
		for(int i = 0 ; i < groups; i++) {
		printf(",%i", kgroup[i]);
		}
		printf("\n");
	}
	}
	printf("obtained cost: %10.4f\n", inst->cost);
}