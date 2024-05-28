#include "../include/tsp_utils.h"



#define SQUARE(x)       (x*x)
/// @brief compute euclidian distance for 2d points
/// @param a instance of point
/// @param b instance of point
/// @return euclidan distance between a and b
double euc_2d(const point a, const point b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y; 

    return sqrt(SQUARE(dx) + SQUARE(dy));
}


/// @brief compute the delta cost between 2 arc switch
/// @param inst instance of TSPinst
/// @param i node i
/// @param in next of i
/// @param j node j
/// @param jn next of j
/// @return result of (c_ij + c_injn) - (c_iin + c_jjn)
double delta_cost(const TSPinst* inst, const unsigned int i, const unsigned int in, const unsigned int j, const unsigned int jn) {

    return  (get_arc(inst, i, j) + get_arc(inst, in, jn)) - 
            (get_arc(inst, i, in) + get_arc(inst, j, jn));
}


/// @brief get distance of the arc i->j
/// @param inst instance of TSPinst
/// @param i node of index i
/// @param j node of index j
/// @return euclidian distance between i and j
double get_arc(const TSPinst* inst, const unsigned int i, const unsigned int j) {
    if(edge_weights == NULL) perror("EDGE WEIGHT not initialized");
    if (i == j) return 0.0;
    int q = coords_to_index(inst->nnodes, i, j);
    if(!edge_weights[q]) {
        edge_weights[q] = euc_2d(inst->points[i], inst->points[j]);
    } 
    return edge_weights[q];
}


/// @brief DEBUGGING function: check if expected cost is equal to real cost recompute
/// @param inst instance of TSPinst 
/// @param tour hamiltonian circuit
/// @param expected_cost expected cost of tour 
void check_tour_cost(const TSPinst* inst, const int* tour, const double expected_cost) {
    double actual_cost = 0;
    
    for(int i = 0; i < inst->nnodes-1; i++) {
        actual_cost += get_arc(inst, tour[i], tour[i+1]); 
    }
    actual_cost += get_arc(inst, tour[0], tour[inst->nnodes-1]);

    if (!abs(expected_cost-actual_cost) > EPSILON) return;
    print_state(Error, "cost_saved and cost_computed differ!");
}


/// @brief recompute cost of a solution (used for transfer result from cplex to heur)
/// @param inst instance of TSPinst
/// @param tmp_sol solution as combination (if NULL => use inst->solution)
/// @return cost of tour;
double compute_cost(TSPinst* inst,const int* tmp_sol) {
    if(tmp_sol == NULL) tmp_sol=inst->solution;

    double out_cost = 0;
    
    for(int i = 0; i < inst->nnodes-1; i++) {
        out_cost += get_arc(inst, tmp_sol[i], tmp_sol[i+1]); 
    }
    out_cost += get_arc(inst, tmp_sol[0], tmp_sol[inst->nnodes-1]);

    return out_cost;
}


cross* set_cross(cross* dest, unsigned int i, unsigned int j, double delta_cost) {
    dest->delta_cost = delta_cost;
    dest->i = i;
    dest->j = j;

    return dest;
}


/// @brief get the nearest available neighbor form an index 
/// @param inst instance of TSPinst
/// @param index starting node
/// @param res list of already viewed points
/// @return return point and distance
near_neighbor get_nearest_neighbor(const TSPinst* inst, const unsigned int index, const int* res) {
    double min = DBL_MAX;
    near_neighbor out = { .dist = 0.0, .index = 0};

    for(int i = 0; i < inst->nnodes; i++) {
        if(res[i] != 0 || i == index) continue;
        
        double dist = get_arc(inst,index,i);
        if (dist < min && dist != 0) {
            out.dist = dist;
            out.index = i;
            min = dist;
        }
    }

    return out;
}


/// @brief check if exists a cross between two arcs, starting from i and j
/// @param inst instance of TSPinst 
/// @param tour hamiltonian circuit 
/// @param i node of index i
/// @param j node of index j
/// @return difference betweem c_ij + c_(i+1)k and c_ik + c_i(i+1)
double check_cross(const TSPinst* inst, const int* tour, const unsigned int i, const unsigned int j) {
    if (i>j) return check_cross(inst, tour, j, i);
    int k = (j+1 == inst->nnodes) ? 0 : j+1;

    return delta_cost(inst, tour[i], tour[i+1], tour[j], tour[k]);
}


/// @brief provide first cross found inside tour
/// @param inst instance of TSPinst 
/// @param tour hamiltonian circuit
/// @return first cross found, if exists, otherwise a cross {-1,-1, EPSILON} 
cross find_first_cross(const TSPinst* inst, const int* tour) {

    for(int i=0; i< inst->nnodes-2; i++) {
        for(int j=i+2; j<inst->nnodes; j++) {
            if (i==0 && j+1==inst->nnodes) continue;

            double delta_cost=check_cross(inst,tour,i,j);
            if(delta_cost < -EPSILON) return (cross){i,j,delta_cost};
        }
    }
    return (cross){-1,-1,EPSILON};
}

void* find_best_cross_job(void* userhandle){
    mt_pars pars = *(mt_pars*) userhandle;
    cross my_best_cross = {-1,-1,INFINITY};

    int my_id;
    for(my_id=0;(pthread_self() != G2OPT_MT_CTX.threads[my_id]) && my_id< G2OPT_MT_CTX.num_threads;my_id++);

    int load = pars.mt_inst->nnodes/G2OPT_MT_CTX.num_threads;
    int end = (my_id != G2OPT_MT_CTX.num_threads-1) ? (my_id+1) + load: pars.mt_inst->nnodes-2;

    for(int i=my_id * load; i<end; i++){
        for(int j=i+2;j<pars.mt_inst->nnodes;j++){
            if(i==0 && j+1==pars.mt_inst->nnodes) continue;
            
            double delta_cost = check_cross(pars.mt_inst,pars.mt_tour,i,j);

            if(delta_cost < my_best_cross.delta_cost + EPSILON && (pars.mt_tabu == NULL || !is_in_tabu(i, j, pars.mt_tabu, pars.mt_tabu_size))){
                //TODO: my_best_cross = (cross){.i=i,.j=j,.delta_cost=delta_cost};
                my_best_cross.i = i;
                my_best_cross.j = j;
                my_best_cross.delta_cost = delta_cost;
            }
        }
    }

    pthread_mutex_lock(&G2OPT_MT_CTX.mutex);
    if(my_best_cross.delta_cost < pars.mt_cross->delta_cost+EPSILON){
                //TODO *pars.mt_cross = (cross){.i=my_best_cross.i,.j=my_best_cross.j,.delta_cost=my_best_cross.delta_cost};
                pars.mt_cross->i = my_best_cross.i;
                pars.mt_cross->j = my_best_cross.j;
                pars.mt_cross->delta_cost = my_best_cross.delta_cost;
    }
    pthread_mutex_unlock(&G2OPT_MT_CTX.mutex);

    return NULL;
}

/// @brief provide cross with max delta cost inside tour 
/// @param inst instance of TSPinst
/// @param tour hamiltonian circuit
/// @return best cross found, if exists, otherwise a cross {-1,-1, EPSILON}
cross find_best_cross(const TSPinst* inst, const int* tour) {

    cross best_cross = {-1,-1,INFINITY};

    mt_pars best_cross_par ={.mt_inst=inst,
                            .mt_tour=tour,
                            .mt_cross=&best_cross,
                            .mt_tabu=NULL,.mt_tabu_size=0};

    int num_threads = (int) log2(inst->nnodes*(inst->nnodes-1)/2);
    run_mt_context(&G2OPT_MT_CTX,num_threads,find_best_cross_job,&best_cross_par);
    return best_cross;
}


/// @brief check if a cross is in tabu
/// @param i node i
/// @param j node j
/// @param tabu array of cross
/// @param tabu_size size of cross array
/// @return 0 if is not in tabu, 1 otherwise
char is_in_tabu(int i, int j, const cross* tabu, const int tabu_size) {

    for(int k = 0; k < tabu_size; k++) {
        if(tabu[k].i == i && tabu[k].j == j) return 1;
    }  

    return 0;
}


/// @brief pick best cross not in tabu
/// @param inst instance of TSPinst
/// @param tour Hamiltonian circuit
/// @param tabu array of cross
/// @param tabu_size size of cross array
/// @return best cross not inside tabu
cross find_best_t_cross(const TSPinst* inst, const int* tour, const cross* tabu, const int tabu_size) {

    cross best_cross = {-1,-1,INFINITY};

    mt_pars best_cross_par ={.mt_inst=inst,
                            .mt_tour=tour,
                            .mt_cross=&best_cross,
                            .mt_tabu=tabu,.mt_tabu_size=tabu_size};

    int num_threads = (int) log2(inst->nnodes*(inst->nnodes-1)/2);
    run_mt_context(&G2OPT_MT_CTX,num_threads,find_best_cross_job,&best_cross_par);
    return best_cross;
}


/// @brief create random changes into a solution
/// @param tour hamiltonian circuit
/// @param size number of nodes inside path
void kick(int* tour, const unsigned int size) {
    int ternary[3] = {-1, -1, -1};
    
    ternary[0] = rand()%size;
    while ((ternary[1] = rand()%size) == ternary[0]);
    while ((ternary[2] = rand()%size) == ternary[0] || ternary[2] == ternary[1]);
    qsort(ternary, 3, sizeof(int), ascending);


    int infuncsol[size];
    memcpy(infuncsol, tour, size * sizeof(int));
    switch (rand()%3)
    {
        case 0:
            for(int c = 0; c < ternary[2] - ternary[1]; c++) infuncsol[ternary[0]+1+c] = tour[ternary[2] - c];
            for(int c = 0; c < ternary[1] - ternary[0]; c++) infuncsol[ternary[0]+ ternary[2] - ternary[1]+1+c] = tour[ternary[0] + 1 + c];
            break;
    
        case 1:
            for(int c = 0; c < ternary[2] - ternary[1]; c++) infuncsol[ternary[0] + 1 + c] = tour[ternary[1] + 1 + c];
            for(int c = 0; c < ternary[1] - ternary[0]; c++) infuncsol[ternary[0] + ternary[2] - ternary[1] + 1 + c] = tour[ternary[0] + 1 + c];
            break;

        case 2:
            for(int c = 0; c < ternary[1] - ternary[0]; c++) infuncsol[ternary[0] + 1 + c] = tour[ternary[1] - c];
            for(int c = 0; c < ternary[2] - ternary[1]; c++) infuncsol[ternary[1] + 1 + c] = tour[ternary[2] - c];
            break;

        default:
            print_state(Error, "Something wrong happen");
            break;
    }
    memcpy(tour, infuncsol, size * sizeof(int));
}


/// @brief print solution associate to TSPinst
/// @param inst instance of TSPinst
/// @param env instance of TSPenv
void print_sol(const TSPinst* inst,const TSPenv* env) {
    printf("\n\e[1mBest Solution Found\e[m (by \e[1m%s\e[m)\n",env->method);
	printf("Cost: \t%10.4f\n", inst->cost);
}


/// @brief print an arc in format x1[%10.4f],y1[%10.4f];x2[%10.4f],y2[%10.4f]
/// @param inst instance of TSPinst
/// @param dest destination string
/// @param i node i
/// @param j node j
/// @return pointer to dest
char* format_arc(const TSPinst* inst,char* dest, const unsigned int i, const unsigned int j) {
    point pi = inst->points[i];
    point pj = inst->points[j];
    sprintf(dest, "%10.4f,%10.4f;%10.4f,%10.4f", pi.x, pi.y, pj.x, pj.y);
    return dest;
}


/// @brief Write on a file the list of arc in a particular format
/// @param inst instance of TSPinst
/// @param dest_file pointer to destination file
void plot_log(const TSPinst* inst, FILE* dest_file) {

    char string[120]; 
    for(int i = 0; i < inst->nnodes - 1; i++) {
        fprintf(dest_file, "%s\n", format_arc(inst, string, inst->solution[i], inst->solution[i+1]));
    }
    fprintf(dest_file, "%s\n", format_arc(inst, string, inst->solution[inst->nnodes-1], inst->solution[0]));
}


/// @brief Write on a file the list of arc in a particular format (from cplex sol)
/// @param inst instance of TSPinst
/// @param ctour tour into cplex format
/// @param dest_file pointer to destination file
void plot_clog(const TSPinst* inst, int* ctour, FILE* dest_file) {
    char string[120]; 
    for(int i = 0; i < inst->nnodes; i++) {
        fprintf(dest_file, "%s\n", format_arc(inst, string, i, ctour[i]));
    }
}