#include "../include/utils.h"

void print_error(const char *err){
    printf("\n\x1b[31mERROR: %s\x1b[0m\n", err); 
    fflush(NULL); 
    exit(1);
} 

uint64_t get_time(){
    return (uint64_t) time(NULL);
}