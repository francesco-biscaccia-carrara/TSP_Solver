#include "../include/utils.h"

void print_error(const char *err){
    printf("\n\n__ERROR: %s\n\n", err); 
    fflush(NULL); 
    exit(1);
} 

uint64_t get_time(){
    return (uint64_t) time(NULL);
}

void help_info(){
    printf("To set the parameters properly you have to execute tsp and add:");
    printf("\n '-in / -f / -file <filename.tsp>' to specity the input file; ");
    printf("\n '-tl / -max_time <time_dbl>' to specity the max execution time (int value);");
    printf("\n '-n / -n_nodes <num_nodes_int>' to specify the number of nodes in the TSP instance (int value);");
    printf("\n '-algo / -method/ -alg <method>' to specify the method to solve the TSP instance;");
    printf("\n '-seed / -rnd_seed <seed>' to specity the random seed (int value);");
    printf("\n '-help / --help / -h' to get help.");
    printf("\n\nNOTICE: you can insert only .tsp file or random seed and number of nodes, NOT BOTH!\n");
}

int coords_to_index(uint32_t n,int i,int j){
    return i<j ? ((i*n-(i-1)*(i)/2) + (j-i-1)-i) : ((j*n-(j-1)*(j)/2) + (i-j-1)-j);
}

double euc_2d(point* a, point* b) {
    double dx = b->x - a->x;
    double dy = b->y - a->y; 

    return sqrt(SQUARE(dx) + SQUARE(dy));
}

