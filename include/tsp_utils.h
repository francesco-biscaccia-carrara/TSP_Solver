#ifndef __TSP_UTILS_H 

#define __TSP_UTILS_H

#define EPSILON     1e-7

#include "tsp.h"

typedef struct{
    unsigned int    i,j;
    double          delta_cost;
} cross;

typedef struct {
    double          dist;
    unsigned int    index;
} near_neighbor;

typedef struct {
    mt_context*         mt_ctx;
    const TSPinst*      mt_inst;
    const int*          mt_tour;
    cross*              mt_cross;
    const cross*        mt_tabu; 
    const int           mt_tabu_size; 
} mt_g2o_pars;


//generic functions
extern double   euc_2d(const point, const point);
extern double   delta_cost(const TSPinst*, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
extern double   get_arc(const TSPinst*, const unsigned int, const unsigned int);
extern void     check_tour_cost(const TSPinst*, const int*, const double);
extern double   compute_cost(TSPinst*,const int*);

//greedy functions
extern near_neighbor get_nearest_neighbor(const TSPinst*, const unsigned int, const char*);

//G2opt functions
extern double   check_cross(const TSPinst*,const int*, const unsigned int, const unsigned int);
extern cross    find_first_cross(const TSPinst*, const int*);
extern cross    find_best_cross(const TSPinst*, const int*);

//TABU functions
extern char     is_in_tabu(int, int, const cross*, const int);
extern cross    find_best_t_cross(const TSPinst*, const int*, const cross*, const int);

//VNS functions
extern double     kick(TSPinst*, int*, const unsigned int);

//Display function
extern void     print_sol(const TSPinst*, const TSPenv*);
extern char*    format_arc(const TSPinst*, char*, const unsigned int, const unsigned int);
extern void     plot_log(const TSPinst*, FILE*);
extern void     plot_clog(const TSPinst*, int*, FILE*);
#endif