#include "../include/tsp_utils.h"


/// @brief compute euclidian distance for 2d points
/// @param a instance of point
/// @param b instance of point
/// @return euclidan distance between a and b
#define SQUARE(x)       (x*x)
double euc_2d(const point a, const point b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y; 

    return sqrt(SQUARE(dx) + SQUARE(dy));
}


/// @brief get distance of the arc i->j
/// @param inst instance of TSPinst
/// @param i node of index i
/// @param j node of index j
/// @return euclidian distance between i and j
double get_arc(TSPinst* inst, const unsigned int i, const unsigned int j) {
    if (i == j) return 0.0;
    int q = coords_to_index(inst->nnodes, i, j);
    if(!inst->edge_weights[q]) {
        inst->edge_weights[q] = euc_2d(inst->points[i], inst->points[j]);
    } 

    return inst->edge_weights[q];
}


/// @brief DEBUGGING function: check if expected cost is equal to real cost recompute
/// @param inst instance of TSPinst 
/// @param tour hamiltonian circuit
/// @param expected_cost expected cost of tour 
void check_tour_cost(TSPinst* inst, const int* tour, const double expected_cost) {
    double actual_cost = 0;
    
    for(int i = 0; i < inst->nnodes-1; i++) {
        actual_cost += get_arc(inst, tour[i], tour[i+1]); 
    }
    actual_cost += get_arc(inst, tour[0], tour[inst->nnodes]);

    if (!abs(expected_cost-actual_cost) > EPSILON) return;
    print_error("cost_saved and cost_computed differ!");
}


/// @brief get the nearest available neighbor form an index 
/// @param inst instance of TSPinst
/// @param index starting node
/// @param res list of already viewed points
/// @return return point and distance
near_neighbor get_nearest_neighbor(TSPinst* inst, const unsigned int index, const int* res) {
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
double check_cross(TSPinst* inst, const int* tour, const unsigned int i, const unsigned int j) {
    if (i>j) return check_cross(inst, tour, j, i);
    int k = (j+1 == inst->nnodes) ? 0 : j+1;

    return  get_arc(inst, inst->solution[i], inst->solution[j]) + get_arc(inst, inst->solution[i+1], inst->solution[k]) -
            get_arc(inst, inst->solution[i], inst->solution[k]) + get_arc(inst, inst->solution[i], inst->solution[i+1]);
}


/// @brief provide first cross found inside tour
/// @param inst instance of TSPinst 
/// @param tour hamiltonian circuit
/// @return first cross found, if exists, otherwise a cross {-1,-1, EPSILON} 
cross find_first_cross(TSPinst* inst, const int* tour) {

    for(int i=0; i< inst->nnodes-2; i++) {
        for(int j=i+2; j<inst->nnodes; j++) {
            if (i==0 && j+1==inst->nnodes) continue;

            double delta_cost=check_cross(inst,tour,i,j);
            if(delta_cost < -EPSILON) return (cross){i,j,delta_cost};
        }
    }
    return (cross){-1,-1,EPSILON};
}


/// @brief provide cross with max delta cost inside tour 
/// @param inst instance of TSPinst
/// @param tour hamiltonian circuit
/// @return best cross found, if exists, otherwise a cross {-1,-1, EPSILON}
cross find_best_cross(TSPinst* inst, const int* tour) {
    cross best_cross = {-1,-1,INFINITY};

    for(int i=0;i<inst->nnodes-2;i++){
        for(int j=i+2;j<inst->nnodes;j++){
            if(i==0 && j+1==inst->nnodes) continue;
            
            double delta_cost = check_cross(inst,tour,i,j);
            if(delta_cost < best_cross.delta_cost + EPSILON){
                best_cross.i = i;
                best_cross.j = j;
                best_cross.delta_cost = delta_cost;
            }
        }
    }
    return best_cross;
}


//TODO: REWRITE THIS FUNCTION
static int comp(const void * elem1, const void * elem2) {
    int f = *((int*) elem1);
    int s = *((int*) elem2);
    if(f > s) return 1;
    if(f < s) return -1;
    return 0;
}


/// @brief create random changes into a solution
/// @param tour hamiltonian circuit
/// @param size number of nodes inside path
//TODO: REWRITE THIS FUNCTIONS
void kick(int* tour, const unsigned int size) {
    int ternary[3] = {-1, -1, -1};
    ternary[0] = rand()%size;
    
    int x = -1;
    while ((x = rand()%size) == ternary[0]);
    ternary[1] = x;    
    while ((x = rand()%size) == ternary[0] || x == ternary[1]);
    ternary[2] = x;

    qsort(ternary, 3, sizeof(int), comp);

    int* infuncsol = (int*) calloc(size, sizeof(int));
    for(int c = 0; c <= ternary[0]; c++) infuncsol[c] = tour[c];
    for(int c = 0; c < ternary[2] - ternary[1]; c++) infuncsol[ternary[0]+1+c] = tour[ternary[2] - c];
    for(int c = 0; c < ternary[1] - ternary[0]; c++) infuncsol[ternary[0]+ ternary[2] - ternary[1]+1+c] = tour[ternary[0] + 1 + c];
    for(int c = ternary[2]+1; c <size; c++) infuncsol[c] = tour[c];
    for(int c = 0; c < size; c++) tour[c] = infuncsol[c];

    if(infuncsol != NULL) free(infuncsol);
}


/// @brief print solution associate to TSPinst
/// @param inst instance of TSPinst
/// @param env instance of TSPenv
void print_sol(const TSPinst* inst,const TSPenv* env) {
    printf("\n\e[1mBest Solution Found\e[m (by \e[1m%s\e[m)\n",env->method);
    //TODO: TMP change. We have to convert CPLEX format sol into our format sol
    if(strcmp(env->method,"CPLEX")) printf("Starting node:\t%i\n",inst->solution[0]);
	printf("Cost: \t%10.4f\n", inst->cost);
}


/// @brief print an arc in format x1.00,y1.00;x2.00,y2.00
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