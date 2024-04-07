#ifndef __CPXS_H 

#define __CPXS_H

#include "tsp.h"

#include <ilcplex/cplex.h>

extern int tsp_CPX_opt(instance *problem);
extern void tsp_blender_loop(instance* problem, cli_info* cli);

#endif

