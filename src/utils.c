#include "../include/utils.h"

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