#include "../include/utils.h"

void print_state(int type, const char* msg, ...) {
    va_list ap;
    char* type_msg = calloc(15, sizeof(char));
    char* type_clr;

    va_start(ap, msg);

    switch (type) {
        case Error:
            type_clr = ANSI_COLOR_RED;
            type_msg = "ERROR";
            break;
    
        case Warn:
            type_clr = ANSI_COLOR_YELLOW;
            type_msg = "WARNING";
            break;
        
        case Info:
            type_clr = ANSI_COLOR_BLUE;
            type_msg = "INFO";
            break;
        default:
            type_clr = ANSI_COLOR_RESET;
            type_msg = "";
            break;
    }

    
    printf("%s\e[1m\e[4m%s\e[0m\e[m%s: ", type_clr, type_msg, type_clr);
    vprintf(msg, ap);
    printf("\x1b[0m");
    va_end(ap);

    if(type == Error) exit(1);
    
}


#define INDEX(n,i,j) (i * (n - 0.5*i - 1.5) + j -1)
/// @brief transform 2d coordinate for a triangular matrix in 1d array
/// @param n number of rows
/// @param i row index
/// @param j column index
/// @return index where the desired value is stored into a 1d array
inline int coords_to_index(const unsigned int n,const int i,const int j){
    if (i == j) print_state(Error, "i == j");
    return i<j ? INDEX(n,i,j) : INDEX(n,j,i);
}


/// @brief get current execution time
/// @return current execution time
double get_time(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    return ((double)tv.tv_sec)+((double)tv.tv_usec/1e+6);
}


/// @brief get elapsed time from certain intial time
/// @param initial_time starting time
/// @return time passed from initial_time
double time_elapsed(const double initial_time) {
    
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return ((double)tv.tv_sec)+((double)tv.tv_usec/1e+6) - initial_time;
}


/// @brief reverse an array of integers passed as a parameter from one index to another. 
/// @param array target array
/// @param from intial index where reverse start
/// @param to terminal index of reverse operation
void reverse(int* array, unsigned int from, unsigned int to){
    while(from<to){
        int tmp = array[from];
        array[from]=array[to];
        array[to]=tmp;
        from++;to--;
    }
}


/// @brief get number of unique integer element inside an array
/// @param inst array of integer 
/// @param size size of array
/// @return number of unque values
int arrunique(const int* inst, const unsigned int size) {
    if(size <= 0) print_state(Error, "input not valid");
    int elem[size];
    elem[0] = inst[0];
    int out = 1;
    char seen = 0;

    for(int i = 0; i < size; i++) {
        for(int j = 0; j < out; j++) {
            if((seen = (elem[j] == inst[i])) != 0) break;
        }

        if(!seen) elem[out++] = inst[i];
        seen = 0;
    }
    return out;
}


/// @brief compare for ascending order
/// @param elem1 first element to compare 
/// @param elem2 second element to compare
/// @return 1 if elem1 > elem2, -1 if elem1 < elem2, 0 otherwise 
int ascending(const void * elem1, const void * elem2) {
    int f = *((int*) elem1);
    int s = *((int*) elem2);
    if(f > s) return 1;
    if(f < s) return -1;
    return 0;
}


/// @brief check if a string is inside an array
/// @param target string to check 
/// @param array array of strings
/// @param array_size size of array
/// @return 0 if target is not in the array, otherwise 1 
char strnin(const char* target,char** array, const size_t array_size){

    size_t wlen = strlen(target);

    for(int i = 0; i < array_size; i++) {
        size_t slen = strlen(array[i]); 
        if(wlen != slen) continue;
        if(!strncmp(target, array[i], slen)) return 1;
    }    
    return 0;
}


/// @brief convert a solution obtained by cplex into solution in heuristic format 
/// @param hsol heuristic solution format
/// @param csol CPLEX solution format
/// @param array_size number of nodes (length of csol)
/// @return pointer where solution is stored
int* cth_convert(int* hsol, int* csol, const unsigned int array_size) {
    int j = csol[0];
	for(int i = 0; i<array_size; i++) {
		hsol[i] = j;
		j = csol[j];
	}

    return hsol;
}


int get_subset_array(int* output, int* succ, int start_point) {
    int i = start_point;
    output[0] = start_point;
    int k = 1;
    while (succ[i] != start_point) { 
        output[k++] = succ[i];
        i = succ[i];
    }

    return k;
}


/// @brief write an array into csv file
/// @param dest destination file 
/// @param content array to write inside csv file
/// @param size length of content
void format_csv_line(FILE* dest, const double* content, const unsigned int size) {
    fprintf(dest, "%f", content[0]);
    for(int i = 1; i < size; i++) {
        fprintf(dest, ",%f",content[i]);
    }
    fprintf(dest, "\n");
}

/// @brief print the execution time to solve TSP problem
/// @param final_time ending time
/// @param init_time starting time
void print_lifespan(const double final_time, const double init_time){
    printf("\n\e[3mTSP problem solved in \e[0m%10.3f sec.s\n", final_time-init_time);
}

/// @brief initialize random seed to prevent strange behaviour
void init_random()  { for(int i=0;i<100;i++) rand();}

void init_mt_context(mt_context* ctx,int num_threads){
    ctx->num_threads=num_threads;
    ctx->threads = (pthread_t*) malloc(num_threads*sizeof(pthread_t));
    pthread_mutex_init(&ctx->mutex, NULL);
}

void assign_task(mt_context* ctx,int th_i,void* (*funct)(void*) ,void* args){
    if(pthread_create(&ctx->threads[th_i],NULL,funct,args)) print_state(Error,"bad tasks assignement!");
}

void delete_mt_context(mt_context* ctx){
    for(int k=0;k<ctx->num_threads;k++) if(pthread_join(ctx->threads[k],NULL))  print_state(Error,"bad join of threads!");
    pthread_mutex_destroy(&ctx->mutex);
    free(ctx->threads);
}

void run_mt_context(mt_context* ctx,int num_threads,void* (*funct)(void*) ,void* args){
    init_mt_context(ctx,num_threads);
    for(int t =0;t<num_threads;t++) assign_task(ctx,t,funct,args);
    delete_mt_context(ctx);
}