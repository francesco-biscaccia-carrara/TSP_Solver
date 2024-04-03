#include "../include/cplex_solver.h"
#include "../include/tsp.h"
#include "../include/utils.h"

#pragma region static_functions

void build_model(instance *problem, CPXENVptr env, CPXLPptr lp){    
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
			double obj = tsp_save_weight(problem,i,j);
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

#pragma endregion


int tsp_CPX_opt(instance *problem){  
    char prob_name[100];
    sprintf(prob_name,"TSP_n-%lu_s-%u",problem->nnodes,problem->random_seed);

	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	CPXLPptr lp = CPXcreateprob(env, &error, prob_name); 

	build_model(problem, env, lp);


	if (  CPXmipopt(env,lp)) print_error("CPXmipopt() error");    
    

	int ncols = CPXgetnumcols(env, lp);
	double sol_cost = 0;
	double *xstar = (double *) calloc(ncols, sizeof(double));
	if (CPXgetx(env, lp, xstar, 0, ncols-1)) print_error("CPXgetx() error");	
	for ( int i = 0; i < problem->nnodes; i++ ){
		for ( int j = i+1; j < problem->nnodes; j++ )
			if (xstar[coords_to_index(problem->nnodes,i,j)] > 0.5) {
				sol_cost += tsp_save_weight(problem,i,j);
				printf("  ... x(%3d,%3d) = 1\n", i+1,j+1);
			}
		
	}

	if(sol_cost < problem->cost){
		//Update solution
		problem->cost = sol_cost;
	}

	free(xstar);
	
	// free and close cplex model   
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env); 

	return 0; // or an appropriate nonzero error code

}


