#include "../include/utils.h"

#define INDEX(n,i,j) (i *n + j - (( i + 1 ) * ( i + 2 )) / 2)

/// @brief transform 2d coordinate for a triangular matrix in 1d array
/// @param n number of rows
/// @param i row
/// @param j column
/// @return index where the desired value is stored
int coords_to_index(size_t n, int i, int j){
    if (i == j) print_error("i == j");
    return i<j ? INDEX(n,i,j) : INDEX(n,j,i);
}

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

FILE* start_plot_pipeline(){
    FILE* gnuplot_pipe = popen("gnuplot -persistent","w");
    fprintf(gnuplot_pipe,"plot '-' with lines linecolor 6\n");
    return gnuplot_pipe;
}

void double_to_plot(FILE* gnuplot_pipe,double cost){
    fprintf(gnuplot_pipe,"%10.4f\n",cost);
}

void close_plot_pipeline(FILE* gnuplot_pipe){
    fprintf(gnuplot_pipe,"e\n");
    pclose(gnuplot_pipe);
}

void check_signal(const int sigint) {
    print_error("Exit...");
}