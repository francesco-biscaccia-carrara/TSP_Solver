#include "../include/tsp_exact.h"
#include "../include/tsp_solver.h"
#include "../include/tsp.h"
#include "../include/utils.h"
#include "../include/mincut.h"


#pragma region static_functions


/// @brief Solve a CPLEX problem (env,lp) 
/// @param env CPLEX environment pointer
/// @param lp CPLEX model pointer
/// @param spare_time remaining time to compute a solution
/// @param lower_bound cost of solution 
/// @param x_star solution in CPX format
static int CPLEX_solve(CPXENVptr* env, CPXLPptr* lp, const double spare_time, double* lower_bound,double* x_star){

	if(spare_time < EPSILON) print_state(Error, "Time limit (%10.4f) is too short!", spare_time);
	CPXsetdblparam(*env,CPX_PARAM_TILIM,spare_time);
	
	if (CPXmipopt(*env,*lp)) print_state(Error, "CPXmipopt() error");

	int STATE = CPXgetstat(*env,*lp);
	
	#if VERBOSE > 1
	switch(STATE){
		case CPXMIP_TIME_LIM_FEAS:  
			print_state(Warn, "Time limit exceeded, but integer solution exists!\n"); //Time limit exceeded, but integer solution exists 
			break; 
		case CPXMIP_TIME_LIM_INFEAS: 		
			print_state(Error, "Time limit exceeded; no integer solution!\n"); //Time limit exceeded; no integer solution 
			break; 		 
		case CPXMIP_INFEASIBLE: 
			print_state(Error, "Solution is integer infeasible!\n"); //Solution is integer infeasible 
			break;
		default: break;	
	}
	#endif

	CPXgetobjval(*env,*lp,lower_bound);
	if (CPXgetx(*env,*lp, x_star, 0, CPXgetnumcols(*env,*lp)-1)) print_state(Error, "CPXgetx() error");

	return STATE;
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
    TSPsol min = { .cost = INFINITY, .tour = NULL };

	//'Warm-up' CPLEX with a feasibile solution given by G2OPT heu
	if(env->warm){
		TSPsol sol = TSPgreedy(inst, ((double)rand())/RAND_MAX*inst->nnodes, TSPg2optb, env->method);   
    	instance_set_solution(inst, sol.tour, sol.cost);

		print_state(Info, "passing an heuristic solution to CPLEX...\n");
		CPLEX_post_heur(&CPLEX_env, &CPLEX_lp, inst->solution, inst->nnodes);
	}

	if(!strncmp(env->method,"BENDER", 6)) min = TSPCbenders(inst, env, &CPLEX_env,&CPLEX_lp, init_time);
	else if(!strncmp(env->method,"BRANCH_CUT", 12)) min = TSPCbranchcut(inst, env, &CPLEX_env,&CPLEX_lp, init_time);
	else { print_state(Error, "No function with alias"); }

	if(min.cost < inst->cost){
		instance_set_solution(inst,min.tour,min.cost);
	}

	double final_time = get_time();
	env->time_exec = final_time - init_time;
	CPLEX_model_delete(&CPLEX_env,&CPLEX_lp);


	#if VERBOSE > 0
		print_lifespan(final_time,init_time);
	#endif
} 


/// @brief use branch&cut to find a solution from LP solution
/// @param inst instance of TSPinst
/// @param tsp_env instance of TSPenvquando lo passi alla callback
/// @param env pointer to CPEXENVptr
/// @param lp pointer to CPEXLPptr
TSPsol TSPCbranchcut(TSPinst* inst, TSPenv* tsp_env, CPXENVptr* env, CPXLPptr* lp, const double start_time) {

    TSPsol out = { .cost = inst->cost, .tour = malloc(inst->nnodes * sizeof(int)) };
	
	//Model has add_SEC_callback installed
	CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;
	if (CPXcallbacksetfunc(*env, *lp, contextid, mount_CUT, inst)) print_state(Error, "CPXcallbacksetfunc() error"); 

	double lb = inst->cost;
	int succ[inst->nnodes];
	int comp[inst->nnodes];
	int nstart[inst->nnodes];
	int ncomp;

	double* x_star = (double*) calloc((inst->nnodes*(inst->nnodes-1))/2, sizeof(double));
	CPLEX_solve(env,lp,tsp_env->time_limit-time_elapsed(start_time),&lb,x_star);
			
	decompose_solution(x_star,inst->nnodes,succ,comp,&ncomp, nstart);
	free(x_star);

	if(ncomp != 1){
		strcpy(tsp_env->method,"B&C-PATCHING");
		patching(inst, succ, comp, ncomp, nstart);
	}

	out.cost = compute_cost(inst,cth_convert(out.tour, succ, inst->nnodes));
	return out;
}


/// @brief use bender's loop to solve a solution from LP solution
/// @param inst instance of TSPinst
/// @param tsp_env instance of TSPenv
/// @param env pointer to CPEXENVptr
/// @param lp pointer to CPEXLPptr
TSPsol TSPCbenders(TSPinst* inst, TSPenv* tsp_env, CPXENVptr* env, CPXLPptr* lp, const double start_time) {

	TSPsol out = { .cost = inst->cost, .tour = malloc(inst->nnodes * sizeof(int)) };
	double lb = inst->cost;
	int succ[inst->nnodes];
	int comp[inst->nnodes];
	int nstart[inst->nnodes];
	int ncomp;
	int iter=0;

	if(tsp_env->time_limit-time_elapsed(start_time) < EPSILON) print_state(Error, "Time limit is too short!");

	while(REMAIN_TIME(start_time, tsp_env)) {
		iter++;

		double* x_star = (double*) calloc((inst->nnodes*(inst->nnodes-1))/2, sizeof(double));
		CPLEX_solve(env,lp,tsp_env->time_limit-time_elapsed(start_time),&lb,x_star);

		#if VERBOSE > 0
			print_state(Info, "Lower-Bound \e[1mBENDER'S LOOP\e[m itereation [%i]: \t%10.4f\n", iter, lb);
		#endif

		decompose_solution(x_star, inst->nnodes, succ, comp, &ncomp, nstart);
		free(x_star);

		//Iter = 0 --> BENDERS reaches the end
		if(ncomp == 1){
			iter=0;
			break;
		}

		//We always apply patching on Benders, in order to have solution if we exceed tl
		add_SEC_mdl(*env,*lp,comp,ncomp,inst->nnodes, succ, nstart);
		patching(inst,succ,comp,ncomp, nstart);
		int* sol  = calloc (inst->nnodes,sizeof(int));
		cth_convert(sol, succ, inst->nnodes);
		CPLEX_edit_post_heur(env,lp,sol,inst->nnodes);
		free(sol);
	}

	
	if(iter) strcpy(tsp_env->method,"BENDERS-PATCHING");
	out.cost = compute_cost(inst,cth_convert(out.tour, succ, inst->nnodes));
	return out;
}


/// @brief patching for a non final bender's loop solution 
/// @param inst instance of TSPinst
/// @param succ solution in cplex format
/// @param comp array that associate edge with route number
/// @param comp_size number of unique element into comp array
void patching(TSPinst* inst, int* succ, int* comp, const unsigned int comp_size, int* nstart) {

	int best_set = 0;
	int group_size = comp_size;

	while(group_size > 1) {
		for(int k1 = 0; k1 < group_size-1; k1++) {
			cross min_cross = { .delta_cost = DBL_MAX, .i = -1, .j = -1 };

			for(int k2 = 0; k2 < group_size; k2++) {
				if(k2 == k1) continue;

				for(int i = nstart[k1]; succ[i] != nstart[k1]; i = succ[i]) {

					for(int j = nstart[k2]; succ[j] != nstart[k2]; j = succ[j]) {
						double del_cost = delta_cost(inst, j, succ[j], i, succ[i]);
						if(del_cost < min_cross.delta_cost) {
							set_cross(&min_cross, i, j, del_cost);
							best_set = k2;
						}
						
					} 
				}
			}

			#if VERBOSE > 2
				printf("\n=============INFO=============\n");
				printf("tour n°: %i\n", comp[nstart[k1]]);
				printf("min:\t%10.4f\ni:\t\t%i\nsucc[i]:\t%i\nj:\t\t%i\nsucc[j]:\t%i\nbest_set:\t%i\n", min_cross.delta_cost, min_cross.i, succ[min_cross.i], min_cross.j, succ[min_cross.j], comp[nstart[best_set]]);
				printf("==============================\n");
			#endif
			
			inst->cost += min_cross.delta_cost;
			int dummy = succ[min_cross.i];
			succ[min_cross.i] = succ[min_cross.j];
			succ[min_cross.j] = dummy;

			for(int z = nstart[best_set]; succ[z] != nstart[best_set]; z = succ[z]) {
				comp[z] = comp[nstart[k1]];
			}
			nstart[best_set] = nstart[--group_size];

			#if VERBOSE > 2
				if(arrunique(succ, inst->nnodes) != inst->nnodes) perror("error in solution building");
				printf("\nremaining tour: %i", comp[nstart[0]]);
				for(int i = 1 ; i < group_size; i++) {
				printf(",%i", comp[nstart[i]]);
				}
				printf("\n");
			#endif
		}
	}
}