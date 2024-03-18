#include "../include/utils.h"

#define FILE_NAME_LEN 100

void print_error(const char *err){
    printf("\n\x1b[31mERROR: %s\x1b[0m\n", err); 
    fflush(NULL); 
    exit(1);
} 

double get_time(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    return ((double)tv.tv_sec)+((double)tv.tv_usec/1e+6);
}

double time_elapsed(double initial_time) {
    
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return ((double)tv.tv_sec)+((double)tv.tv_usec/1e+6) - initial_time;
}

void save_cost_on_file(size_t nnodes,uint32_t seed,double cost){
    char file_name[FILE_NAME_LEN];
    sprintf(file_name,"tabu_costs/.cost_tabu_n_%lu_s_%u.dat",nnodes,seed);
   
    char command[2*FILE_NAME_LEN];
    sprintf(command,"echo '%10.4f'>> '%s'",cost,file_name);
    system(command);
}
