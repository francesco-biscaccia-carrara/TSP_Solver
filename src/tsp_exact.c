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

	if(spare_time < EPSILON) print_state(Error, "Time limit is too short!");
	CPXsetdblparam(*env,CPX_PARAM_TILIM,spare_time);
	
	if (CPXmipopt(*env,*lp)) print_state(Error, "CPXmipopt() error");

	int STATE = CPXgetstat(*env,*lp);
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

	//'Warm-up' CPLEX with a feasibile solution given by G2OPT heu
	if(env->warm){
		TSPgreedy(inst, ((double)rand())/RAND_MAX*inst->nnodes, TSPg2optb, env->method);

		print_state(Info, "passing an heuristic solution to CPLEX...\n");
		CPLEX_post_heur(&CPLEX_env, &CPLEX_lp, inst->solution, inst->nnodes);
	}

	if(!strncmp(env->method,"BENDER", 6)) TSPCbenders(inst, env, &CPLEX_env,&CPLEX_lp);
	else if(!strncmp(env->method,"BRANCH_CUT", 12)) TSPCbranchcut(inst, env, &CPLEX_env,&CPLEX_lp);
	else { print_state(Error, "No function with alias"); }

	double final_time = get_time();
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
void TSPCbranchcut(TSPinst* inst, TSPenv* tsp_env, CPXENVptr* env, CPXLPptr* lp) {

	//Model has add_SEC_callback installed
	CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION;
	if (CPXcallbacksetfunc(*env, *lp, contextid, mount_CUT, inst)) print_state(Error, "CPXcallbacksetfunc() error");

	double start_time = get_time(); 
	double lb = inst->cost;

	int *succ = calloc(inst->nnodes,sizeof(int));
	int *comp = calloc(inst->nnodes,sizeof(int));
	int *nstart = calloc(inst->nnodes, sizeof(int));
	int ncomp;

	double* x_star = (double*) calloc((inst->nnodes*(inst->nnodes-1))/2, sizeof(double));
	CPLEX_solve(env,lp,tsp_env->time_limit-time_elapsed(start_time),&lb,x_star);
			
	decompose_solution(x_star,inst->nnodes,succ,comp,&ncomp, nstart);
	free(x_star);

	if(lb < inst->cost){
		int* sol  = calloc(inst->nnodes, sizeof(int));
		if(ncomp != 1){
			strcpy(tsp_env->method,"B&C-PATCHING");
			patching(inst, succ, comp, ncomp, nstart);
		} 

		double cost = compute_cost(inst,cth_convert(sol, succ, inst->nnodes));
		if(cost < inst->cost) {
			instance_set_solution(inst,sol,cost);
		}
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
	int* nstart = calloc(inst->nnodes, sizeof(int));
	int ncomp;
	int iter=0;

	double start_time = get_time(); 
	if(tsp_env->time_limit-time_elapsed(start_time)<EPSILON) print_state(Error, "Time limit is too short!");

	while(time_elapsed(start_time) < tsp_env->time_limit) {
		iter++;

		double* x_star = (double*) calloc((inst->nnodes*(inst->nnodes-1))/2, sizeof(double));
		CPLEX_solve(env,lp,tsp_env->time_limit-time_elapsed(start_time),&lb,x_star);

		#if VERBOSE > 0
			printf("Lower-Bound \e[1mBENDER'S LOOP\e[m itereation [%i]: \t%10.4f\n", iter, lb);
		#endif

		decompose_solution(x_star,inst->nnodes,succ,comp,&ncomp, nstart);
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
		CPLEX_post_heur(env,lp,succ,inst->nnodes);
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
void patching(TSPinst* inst, int* succ, int* comp, const unsigned int comp_size, int* nstart) {

	int best_i = 0, best_j = 0;
	int best_set = 0;

	int group_size = comp_size;

	while(group_size > 1) {
		for(int k1 = 0; k1 < group_size-1; k1++) {
			double min = DBL_MAX;
			for(int k2 = k1+1; k2 < group_size; k2++) {

				int i = nstart[k1];
				int j = nstart[k2];
				while (succ[i] != nstart[k1]) {
					while (succ[j] != nstart[k2]) {

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
				printf("tour n°: %i\n", comp[nstart[k1]]);
				printf("min:\t%10.4f\ni:\t\t%i\nsucc[i]:\t%i\nj:\t\t%i\nsucc[j]:\t%i\nbest_set:\t%i\n", min, best_i, succ[best_i], best_j, succ[best_j], comp[nstart[best_set]]);
				printf("==============================\n");
			#endif

			inst->cost += min;
			int dummy = succ[best_i];
			succ[best_i] = succ[best_j];
			succ[best_j] = dummy;

			int z = nstart[best_set]; 
			while(succ[z] != nstart[best_set]) {
				comp[z] = comp[nstart[k1]];
				z = succ[z];
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