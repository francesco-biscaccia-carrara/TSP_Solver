#include "../include/algorithm.h"
#include "../include/tsp.h"
#include "../include/mt.h"
#include "../include/utils.h"

#define TABU_SIZE 500
static mt_context mt_g2opt_b;

#pragma region static_functions

static void reverse(int* solution, int i, int j){
    while(i<j){
        int tmp = solution[i];
        solution[i]=solution[j];
        solution[j]=tmp;
        i++;j--;
    }
}

static point_n_dist get_min_distance_point(int index, instance *problem, int* res) {

    double min = DBL_MAX;
    point_n_dist out = { .dist = 0.0, .index = 0};

    for(int i = 0; i < problem->nnodes; i++) {
        if(res[i] != 0) continue;
        
        double dist = tsp_save_weight(problem,index,i);

        if (dist < min && dist != 0) {
            out.dist = dist;
            out.index = i;
            min = dist;
        }
    }

    return out;
}

static void check_path_cost(int* tmp_sol,double tmp_cost,instance* problem){

    double cost_saved = tmp_cost;
    double cost_computed =0;
    
    for(int i=0;i< problem->nnodes-1;i++){
        cost_computed+=tsp_save_weight(problem,tmp_sol[i],tmp_sol[i+1]);
    }
    cost_computed+=tsp_save_weight(problem,tmp_sol[problem->nnodes-1],tmp_sol[0]);
    if (cost_saved - cost_computed > EPSILON){
        print_error("cost_saved and cost_computed differ!");
    }
}

static double check_cross(instance* problem,int* tmp_sol,int i,int j){
    int k = (j+1 == problem->nnodes) ? 0 : j+1;

    return  (tsp_save_weight(problem,tmp_sol[i],tmp_sol[j]) + tsp_save_weight(problem,tmp_sol[i+1],tmp_sol[k])) - 
            (tsp_save_weight(problem,tmp_sol[i],tmp_sol[i+1]) + tsp_save_weight(problem,tmp_sol[j],tmp_sol[k]));
}

static cross find_first_cross(int* tmp_sol,instance* problem){

    for(int i=0;i<problem->nnodes-2;i++){
        for(int j=i+2;j<problem->nnodes;j++){
            if(i==0 && j+1==problem->nnodes) continue;
            double delta_cost=check_cross(problem,tmp_sol,i,j);
            if(delta_cost < -EPSILON) return (cross){i,j,delta_cost};
        }
    }
    return (cross){-1,-1,EPSILON};
}

static cross find_best_cross(int* tmp_sol,instance* problem){
    cross best_cross = {-1,-1,INFINITY};

    for(int i=0;i<problem->nnodes-2;i++){
        for(int j=i+2;j<problem->nnodes;j++){
            if(i==0 && j+1==problem->nnodes) continue;
            double delta_cost=check_cross(problem,tmp_sol,i,j);
            if(delta_cost < best_cross.delta_cost + EPSILON){
                best_cross.i = i;
                best_cross.j = j;
                best_cross.delta_cost = delta_cost;
            }
        }
    }
    return best_cross;
}

static char is_not_in_tabu(int i,int j, int k, int h, move* tabu) {

    for(int u = 0; u < TABU_SIZE; u++){
        
        if(tabu[u].i == i &&
           tabu[u].j == j &&
           tabu[u].k == k &&
           tabu[u].h == h)
            return 0;
           
    }

    return 1;
}

static move find_best_cross_tabu(int* tmp_sol, instance* problem, move* tabu){
    move best_move = {-1,-1,-1,-1, INFINITY};

    for(int i=0;i<problem->nnodes-2;i++){
        for(int j=i+2;j<problem->nnodes;j++){
            if(i==0 && j+1==problem->nnodes) continue;
            double delta_cost=check_cross(problem,tmp_sol,i,j);
            if(delta_cost < best_move.delta_cost+EPSILON && is_not_in_tabu(i, j, i+1, j+1, tabu)){
                best_move.i = i;
                best_move.j = j;
                best_move.k = i+1;
                best_move.h = j+1;
                best_move.delta_cost = delta_cost;
            }
        }
    }

    if(best_move.delta_cost == INFINITY) print_error("ALL POSSIBLE MOVE IN THE TABU");

    return best_move;
} 

int comp(const void * elem1, const void * elem2) {
    int f = *((int*) elem1);
    int s = *((int*) elem2);
    if(f > s) return 1;
    if(f < s) return -1;
    return 0;
}

static void kick(int* tmp_sol, int size) {
    int ternary[3] = {-1, -1, -1};
    ternary[0] = rand()%size;
    
    int x = -1;
    while ((x = rand()%size) == ternary[0]);
    ternary[1] = x;    
    while ((x = rand()%size) == ternary[0] || x == ternary[1]);
    ternary[2] = x;

    qsort(ternary, 3, sizeof(int), comp);

    int* infuncsol = (int*) calloc(size, sizeof(int));
    for(int c = 0; c <= ternary[0]; c++) infuncsol[c] = tmp_sol[c];
    for(int c = 0; c < ternary[2] - ternary[1]; c++) infuncsol[ternary[0]+1+c] = tmp_sol[ternary[2] - c];
    for(int c = 0; c < ternary[1] - ternary[0]; c++) infuncsol[ternary[0]+ ternary[2] - ternary[1]+1+c] = tmp_sol[ternary[0] + 1 + c];
    for(int c = ternary[2]+1; c <size; c++) infuncsol[c] = tmp_sol[c];
    for(int c = 0; c < size; c++) tmp_sol[c] = infuncsol[c];

    if(infuncsol != NULL) free(infuncsol);
}

#pragma region mt_function

static void* find_local_best_swap(void* data){
    mt_data_g2pot_b* d = (mt_data_g2pot_b*) data;
    instance* prob = (instance*) d->prob;
    cross * curr_cross = (cross*) d->best_cross; 

    int start = d->k * prob->nnodes/mt_g2opt_b.num_threads - d->k;
    int end = ((d->k)+1)* prob->nnodes/mt_g2opt_b.num_threads-1 < prob->nnodes-2 ? ((d->k)+1)* prob->nnodes/mt_g2opt_b.num_threads-1 : prob->nnodes-2 ;

    cross best_cross = {-1,-1,INFINITY};
    for(int i=start;i<end;i++){
        for(int j=i+2;j<prob->nnodes;j++){
            if(i==0 && j+1==prob->nnodes) continue;
            double delta_cost=check_cross(prob,d->tmp_sol,i,j);
            if(delta_cost < best_cross.delta_cost+EPSILON){
                best_cross.i=i;
                best_cross.j=j;
                best_cross.delta_cost= delta_cost;
            }
        }
    }
    pthread_mutex_lock(&mt_g2opt_b.mutex);
    if(best_cross.delta_cost < curr_cross->delta_cost+EPSILON){
                curr_cross->i = best_cross.i;
                curr_cross->j = best_cross.j;
                curr_cross->delta_cost = best_cross.delta_cost;
    }
    pthread_mutex_unlock(&mt_g2opt_b.mutex);
    return NULL;
}

static cross find_best_cross_mt(int* tmp_sol,const instance* problem){
    cross best_cross = {-1,-1,INFINITY};
    
    init_mt_context(&mt_g2opt_b,sugg_num_threads(problem->nnodes));
    mt_data_g2pot_b data_array[mt_g2opt_b.num_threads];

    for (int k = 0; k <mt_g2opt_b.num_threads; k++) {
        data_array[k].prob = problem;
        data_array[k].tmp_sol = tmp_sol;
        data_array[k].best_cross = &best_cross;
        data_array[k].k=k;
        assign_task(&mt_g2opt_b,k,find_local_best_swap,&data_array[k]);
    }
    delete_mt_context(&mt_g2opt_b);
    return best_cross;
}

#pragma endregion

#pragma endregion



void solve_heuristic (cli_info* cli_info, instance* problem) {

    //GET OPTIMIZATION FUNCTION FOR GREEDY
    void *opt_func;
    if (!strncmp(cli_info->method,"GREEDY",5) || 
        !strncmp(cli_info->method,"TABU_R",6) ||
        !strncmp(cli_info->method,"VNS",3)) {
        opt_func = NULL;
    }
    else if(!strncmp(cli_info->method,"G2OPT_F",7)) {
        opt_func = tsp_g2opt;
    }
    else if(!strncmp(cli_info->method,"G2OPT_B",7) || 
            !strncmp(cli_info->method,"TABU_B",6)) {
        if(cli_info->mt) opt_func = tsp_g2opt_best_mt;
        else opt_func = tsp_g2opt_best;
    }
    else {
        print_error("No function with alias");
    }
    
    double initial_time = get_time();

    for(int i=0;i < problem->nnodes && time_elapsed(initial_time) <= cli_info->time_limit; i++) {
        tsp_greedy(i,problem, opt_func, cli_info->method);
    }

    if (!strncmp(cli_info->method,"TABU_B",6) ||
        !strncmp(cli_info->method,"TABU_R",6)) {
        tabu_search(problem, initial_time, cli_info);
        }
    if (
        !strncmp(cli_info->method,"VNS",5)) {
        VNS(problem, initial_time, cli_info);
    }

    double end_time = get_time();

    #if VERBOSE > 0
	printf("TSP problem solved in %10.4f sec.s\n", end_time-initial_time);
	#endif
}

void tsp_greedy(int index, instance* problem, void (opt_func)(int*, double*, instance*), char* opt_func_name) {

    double cost = 0;
    int current_index = index;
    point_n_dist new_point;

    int* used_node = calloc(problem->nnodes, sizeof(int));
    int* result = malloc(problem->nnodes * sizeof(int));

    result[0] = current_index;
    used_node[index] = 1;

    for (int i = 1; i < problem->nnodes; i++) {  
        new_point = get_min_distance_point(result[i-1], problem, used_node);
        
        cost += new_point.dist;
        used_node[new_point.index] = 1;
        result[i] = new_point.index;
    }

    cost +=tsp_save_weight(problem, new_point.index, index);    
    free(used_node);

    #if VERBOSE > 1
    printf("Partial \e[1mGREEDY\e[m solution starting from [%i]: \t%10.4f\n", index, cost);
    #endif

    if(opt_func != NULL) {
        opt_func(result,&cost,problem);

        #if VERBOSE > 1
        printf("Partial \e[1m%s\e[m solution starting from [%i]: \t%10.4f\n",opt_func_name, index, cost);
        #endif
    }
    
     
    if(cost < problem->cost){
        free(problem->solution);

        problem->cost = cost;
        problem->solution = result;
    }else{
        free(result);
    }
}

void tsp_g2opt(int* tmp_sol, double* cost, instance* problem){
    char improve = 1;
    while (improve) {
    cross curr_cross = find_first_cross(tmp_sol,problem);

    if(curr_cross.delta_cost >= -EPSILON) {
        improve = 0;
    }
    else {
        reverse(tmp_sol,curr_cross.i+1,curr_cross.j);
        *cost+=curr_cross.delta_cost;
        #if VERBOSE > 2
        check_path_cost(tmp_sol,*cost,problem);
        #endif
    }
  }
}

void tsp_g2opt_best(int* tmp_sol, double* cost, instance* problem){
  char improve = 1;
  
  while (improve) {
    cross curr_cross = find_best_cross(tmp_sol,problem);

    if(curr_cross.delta_cost >= -EPSILON) {
        improve = 0;
    }
    else {
        reverse(tmp_sol,curr_cross.i+1,curr_cross.j);
        *cost+=curr_cross.delta_cost;
        
        #if VERBOSE > 2
        check_path_cost(tmp_sol,*cost,problem);
        #endif
    }
  }
}

void tsp_g2opt_best_mt(int* tmp_sol, double* cost, instance* problem){
  char improve = 1;
  
  while (improve) {
    cross curr_cross = find_best_cross_mt(tmp_sol,problem);

    if(curr_cross.delta_cost >= EPSILON) {
        improve = 0;
    }
    else {
        reverse(tmp_sol,curr_cross.i+1,curr_cross.j);
        *cost+=curr_cross.delta_cost;
        
        #if VERBOSE > 2
        check_path_cost(tmp_sol,*cost,problem);
        #endif
    }
  }
}

void tabu_search(instance* problem, double initial_time, cli_info* cli_info) {
    move *tabu_table = (move*) malloc(TABU_SIZE * sizeof(move));

    //reinizialize current solution
    int* tmp_sol = malloc(sizeof(int) * problem->nnodes);
    for(int i = 0; i < problem->nnodes; i++)   
        tmp_sol[i] = problem->solution[i];

    double cost = problem->cost;
    int tabu_index = 0;

    FILE* pipe = start_plot_pipeline();

    while (time_elapsed(initial_time) <= cli_info->time_limit) {

        move m = find_best_cross_tabu(tmp_sol,problem, tabu_table);
        
        #if VERBOSE > 2
        printf("delta cost:\t%10.4f\n",  m.delta_cost);
        #endif

        cost += m.delta_cost;
        reverse(tmp_sol,m.i+1,m.j);

        double_to_plot(pipe,cost);

        if(m.delta_cost >= EPSILON) {
            tabu_table[tabu_index % TABU_SIZE] = m;
            tabu_index++;
        }
        else{
            if(cost < problem->cost) {
                problem->cost = cost;
                printf("new_cost:\t%10.4f\n", problem->cost);
                
                for(int i = 0; i < problem->nnodes; i++)    
                    problem->solution[i] = tmp_sol[i];

                #if VERBOSE > 2
                check_path_cost(tmp_sol,*cost,problem);
                #endif
            }
        }         
    }
    close_plot_pipeline(pipe);
}

void VNS(instance* problem, double initial_time, cli_info* cli_info) {
    
    double cost = problem->cost;
    int* tmp_sol = malloc(sizeof(int) * problem->nnodes);
    memcpy(tmp_sol, problem->solution, problem->nnodes * sizeof(problem->solution[0]));
    

    while (time_elapsed(initial_time) <= cli_info->time_limit)
    {
        tsp_g2opt_best(tmp_sol, &cost, problem);
        if(cost < problem->cost) {
            problem->cost = cost;
            memcpy(problem->solution, tmp_sol, problem->nnodes * sizeof(problem->solution[0]));
            
            #if VERBOSE > 0
            printf("new best cost:\t%10.4f\n", problem->cost);
            #endif
        }
        for(int i = 0; i < 4; i++) kick(tmp_sol, problem->nnodes);
        
        cost = 0;
        for (size_t i = 0; i < problem->nnodes - 1; i++) {
            cost += tsp_save_weight(problem, tmp_sol[i],tmp_sol[i+1]);
        }
        cost +=tsp_save_weight(problem, tmp_sol[problem->nnodes-1], tmp_sol[0]);
    }

    free(tmp_sol);
}