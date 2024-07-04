#ifndef __TSP_EUTILS_H 

#define __TSP_EUTILS_H

#include "tsp_solver.h"
#include "mincut.h"
#include <ilcplex/cplex.h>

typedef struct {
    CPXCALLBACKCONTEXTptr  context;
    unsigned int    nnodes;
} cut_par;



extern void             CPLEX_model_new(TSPinst*, CPXENVptr*, CPXLPptr*);
extern void             CPLEX_model_delete(CPXENVptr*, CPXLPptr*);
extern void             CPLEX_log(CPXENVptr*, const TSPenv*);

extern void             decompose_solution(const double*, const unsigned int, int*, int*, int*, int*);
extern void             CPLEX_post_heur(CPXENVptr, CPXLPptr, int*, const unsigned int);
extern void             CPLEX_edit_post_heur(CPXENVptr*, CPXLPptr*, int*, const unsigned int);

//callback
extern void             add_SEC_mdl(CPXCENVptr, CPXLPptr,const int* , const unsigned int, const unsigned int, int*, int* );
extern int              add_SEC_int(CPXCALLBACKCONTEXTptr, TSPinst);
extern int              add_SEC_flt(CPXCALLBACKCONTEXTptr, TSPinst);
extern int CPXPUBLIC    mount_CUT(CPXCALLBACKCONTEXTptr, CPXLONG, void*);

extern void             add_warm_start(CPXENVptr, CPXLPptr, TSPinst*, TSPenv*, char*);
extern void             patching(TSPinst*, int*, int*, const unsigned int, int*);


#endif